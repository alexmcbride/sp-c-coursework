// Cwk2: client.c - message length headers with variable sized payloads
//  also use of readn() and writen() implemented in separate code module

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "rdwrn.h"

#define PORT_NUMBER 50001
#define INPUTSIZ 256

void send_request(int sockfd, int request_code);
size_t get_message(int sockfd);

int main(void)
{
    // *** this code down to the next "// ***" does not need to be changed except the port number
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Error - could not create socket");
	exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;

    // IP address and port of server we want to connect to
    serv_addr.sin_port = htons(PORT_NUMBER);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // try to connect...
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  
    {
		perror("Error - connect failed\n");
		exit(1);
    } 
    else
    {
       printf("Connected to server...\n");
    }

    // get a string from the server
    get_message(sockfd);

    // show menu and have user select option
    int running = 1;
    while (running)
    {
    	int input;
    	printf("Main menu:\n");
    	printf("1. Message\n");
    	printf("2. Quit\n");
    	printf("Choose option: ");
		scanf("%d", &input);

    	switch (input) {
    		case 1:
    			send_request(sockfd, input);
    			get_message(sockfd);
    		break;
    		case 2:
    			printf("Now exiting!\n");
				running = 0;
    		break;
    		default:
    			fprintf(stderr, "%s", "Error: invalid input\n");
    		break;
    	}
    }

    // make sure sockets are cleaned up
    close(sockfd);
    exit(EXIT_SUCCESS);
}

void send_request(int sockfd, int request_code)
{
	writen(sockfd, (unsigned char *)&request_code, sizeof(int));
}

size_t get_message(int sockfd)
{
    char message[INPUTSIZ];
    size_t length;

    readn(sockfd, (unsigned char *) &length, sizeof(size_t));	
    readn(sockfd, (unsigned char *) message, length);

    printf("Message: %s\n", message);
    printf("Received: %zu bytes\n\n", length);

    return length;
} 
