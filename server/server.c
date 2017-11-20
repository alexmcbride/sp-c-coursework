// Cwk2: server.c - multi-threaded server using readn() and writen()

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "rdwrn.h"

// Constants
#define PORT_NUMBER 50031
#define INPUTSIZ 256
#define STUDENT_ID "S1715224"

// Prototypes
void *client_handler(void *);
void handle_student_id(int connfd);
void handle_server_time(int connfd);
void send_message(int socket, char *msg);
void get_ip_address(char *ip_str);
void die(char *error);

// Functions
int main(void)
{
    int listenfd = 0, connfd = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t socksize = sizeof(struct sockaddr_in);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER);

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (listen(listenfd, 10) == -1) {
	    die("Error - failed to listen");
    }
    // end socket setup

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    while (1) {
	    printf("Waiting for a client to connect...\n");
	    connfd = accept(listenfd, (struct sockaddr *) &client_addr, &socksize);
	    printf("Connection accepted...\n");

	    pthread_t sniffer_thread;
            // third parameter is a pointer to the thread function, fourth is its actual parameter
	    if (pthread_create(&sniffer_thread, NULL, client_handler, (void *) &connfd) < 0) {
	        die("Error - could not create thread");
	    }

	    //Now join the thread , so that we dont terminate before the thread
	    //pthread_join( sniffer_thread , NULL);
	    printf("Handler assigned\n");
    }

    // never reached...
    // ** should include a signal handler to clean up
    // shutdown(listenfd, SHUT_RDWR);
    // close(listenfd);
    exit(EXIT_SUCCESS);
} 

void die(char *error)
{
    puts(error);
    exit(EXIT_FAILURE);
}

void get_ip_address(char *ip_str) 
{
    int fd;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));

    // Get IP addr from struct
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    // Copy result into parameter
    strcpy(ip_str, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

void handle_student_id(int connfd)
{
    // Get IP address
    char ip_str[INET_ADDRSTRLEN];
    memset(ip_str, 0, INET_ADDRSTRLEN);
    get_ip_address(ip_str);

    // Concat string
    char str[24];
    sprintf(str, "%s:%s", ip_str, STUDENT_ID);

    // Send to client
    send_message(connfd, str);
}

void handle_server_time(int connfd)
{
    // Get time.
    time_t t;
    if ((t = time(NULL)) == -1) {
        die("Error - could not get time");
    }

    // Convert to local time.
    struct tm *tm;
    if ((tm = localtime(&t)) == NULL) {
        die("Error - could not get localtime");
    }

    // Get time string.
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%c", tm);

    // Send time message to client.
    send_message(connfd, time_str);
}

void *client_handler(void *socket_desc)
{
    int connfd = *(int *) socket_desc;

    // Send welcome message to client.
    send_message(connfd, "Welcome to the server!");

    while (1) 
    {
        int request_code;
        int count = readn(connfd, (unsigned char *) &request_code, sizeof(int)); 

        // Check if client disconnected.
        if (count == 0) 
        {
            printf("Error - lost client connection\n");
            break;
        }
        else if (count < 0)
        {
           printf("Error - client read error: %d\n", count);
           break;
        }

        // Handle client requests
        switch (request_code) 
        {
            case 1:
                handle_student_id(connfd);
            break;
            case 2:
                handle_server_time(connfd);
            break;
        }
    }

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    // always clean up sockets gracefully
    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    return 0;
}  

void send_message(int socket, char *msg)
{
    size_t length = strlen(msg) + 1; // Add one to account for NULL terminator
    writen(socket, (unsigned char *) &length, sizeof(size_t));   
    writen(socket, (unsigned char *) msg, length);    
} 
