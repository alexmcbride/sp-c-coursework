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

// Constants
#define HOST_ADDRESS "127.0.0.1"
#define PORT_NUMBER 50031
#define INPUTSIZ 256

// Prototypes
void handle_server(int sockfd);
void send_request(int sockfd, int request_code);
size_t get_message(int sockfd);
size_t read_socket(int sockfd, unsigned char *buffer, int length);
size_t write_socket(int sockfd, unsigned char *buffer, int length);
void die(char *message);

// Functions
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
    serv_addr.sin_addr.s_addr = inet_addr(HOST_ADDRESS);

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

    // Handle communication with the server.
    handle_server(sockfd);

    // make sure sockets are cleaned up
    close(sockfd);
    exit(EXIT_SUCCESS);
}

int show_menu() 
{
	printf("Main menu:\n");
	printf("1. Get student ID\n");
	printf("2. Get server time\n");
	printf("3. Quit\n");
	printf("Choose option: ");

	int input;	
	scanf("%d", &input);

	return input;
}

void handle_server(int sockfd)
{
	// get welcome message from the server
    get_message(sockfd);

    // show menu and have user select option
    int running = 1;
    while (running)
    {
    	int input = show_menu();
    	switch (input) {
    		case 1:
    			send_request(sockfd, 1);
    			get_message(sockfd);
    		break;
    		case 2:
    			send_request(sockfd, 2);
    			get_message(sockfd);
    		break;    		
    		case 3:
    			printf("Now exiting!\n");
				running = 0;
    		break;
    		default:
    			fprintf(stderr, "%s", "Error - invalid input\n");
    		break;
    	}
    }
}

void send_request(int sockfd, int request_code)
{
	write_socket(sockfd, (unsigned char *)&request_code, sizeof(int));
}

size_t write_socket(int sockfd, unsigned char *buffer, int length)
{
	size_t result = writen(sockfd, buffer, length);
	if (result <= 0)
	{
		close(sockfd);
		die("Error - read socket error");
	}
	return result;
}

void die(char *message) 
{
	puts(message);
	exit(EXIT_FAILURE);
}

size_t read_socket(int sockfd, unsigned char *buffer, int length) 
{
	size_t result = readn(sockfd, (unsigned char *)buffer, length);
	if (result == 0) 
	{
		close(sockfd);
		die("Error - connection lost");
	}
	else if (result < 0)
	{
		close(sockfd);
		die("Error - read socket error");
	}
	return result;
}

size_t get_message(int sockfd)
{    
	size_t length;
    char message[INPUTSIZ];

    read_socket(sockfd, (unsigned char *) &length, sizeof(size_t));	
    read_socket(sockfd, (unsigned char *) message, length);

    printf(">> %s\n", message);

    return length;
} 
