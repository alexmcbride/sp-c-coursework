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

// thread function
void *client_handler(void *);

typedef struct {
    int id_number;
    int age;
    float salary;
} employee;

void get_and_send_employee(int, employee *);
void send_hello(int);

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
    serv_addr.sin_port = htons(50001);

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
	connfd =
	    accept(listenfd, (struct sockaddr *) &client_addr, &socksize);
	printf("Connection accepted...\n");

	pthread_t sniffer_thread;
        // third parameter is a pointer to the thread function, fourth is its actual parameter
	if (pthread_create
	    (&sniffer_thread, NULL, client_handler,
	     (void *) &connfd) < 0) {
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
} // end main()

// thread function - one instance of each for each connected client
// this is where the do-while loop will go
void *client_handler(void *socket_desc)
{
    //Get the socket descriptor
    int connfd = *(int *) socket_desc;

    send_hello(connfd);

    employee *employee1;
    employee1 = (employee *) malloc(sizeof(employee));

    int i;
    for (i = 0; i < 5; i++) {
	printf("(Counter: %d)\n", i);
	get_and_send_employee(connfd, employee1);
	printf("\n");
    }

    free(employee1);

    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    // always clean up sockets gracefully
    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    return 0;
}  // end client_handler()

// how to send a string
void send_hello(int socket)
{
    char hello_string[] = "hello SP student";

    size_t n = strlen(hello_string) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	
    writen(socket, (unsigned char *) hello_string, n);	  
} // end send_hello()

// as before...
void get_and_send_employee(int socket, employee * e)
{
    size_t payload_length;

    size_t n =
	readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    printf("payload_length is: %zu (%zu bytes)\n", payload_length, n);
    n = readn(socket, (unsigned char *) e, payload_length);

    printf("Age is %d\n", e->age);
    printf("id is %d\n", e->id_number);
    printf("Salary is %6.2f\n", e->salary);
    printf("(%zu bytes)\n", n);

    // make arbitrary changes to the struct & then send it back
    e->age++;
    e->salary += 1.0;

    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));
    writen(socket, (unsigned char *) e, payload_length);
}  // end get_and_send_employee()
