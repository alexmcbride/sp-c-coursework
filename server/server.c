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
#include "rdwrn.h"

#define PORT_NUMBER 50001
#define INPUTSIZ 256

// thread function
void *client_handler(void *);
void send_message(int socket, char *msg);

// you shouldn't need to change main() in the server except the port number
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
	    perror("Failed to listen");
	    exit(EXIT_FAILURE);
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
	        perror("could not create thread");
	        exit(EXIT_FAILURE);
	    }

	    //Now join the thread , so that we dont terminate before the thread
	    //pthread_join( sniffer_thread , NULL);
	    printf("Handler assigned\n");
    }

    // never reached...
    // ** should include a signal handler to clean up
    exit(EXIT_SUCCESS);
} 

void handle_request(int connfd, int request_code)
{
    printf("Handling request code: %d\n", request_code);

    switch (request_code) {
        case 1:
            send_message(connfd, "Response to request code 1!");
        break;
    }
}

void *client_handler(void *socket_desc)
{
    //Get the socket descriptor
    int connfd = *(int *) socket_desc;

    send_message(connfd, "Welcome, user!");

    while (1) {
        int request_code;
        int count = readn(connfd, (unsigned char *) &request_code, sizeof(int)); 

        // Check if client disconnected.
        if (count == 0) {
            break;
        }

        handle_request(connfd, request_code);
    }

    // Cleanup...
    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    // always clean up sockets gracefully
    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    return 0;
}  

void send_message(int socket, char *msg)
{
    size_t length = strlen(msg) + 1;
    writen(socket, (unsigned char *) &length, sizeof(size_t));   
    writen(socket, (unsigned char *) msg, length);    
} 
