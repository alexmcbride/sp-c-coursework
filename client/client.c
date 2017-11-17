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

struct packet
{
    int length;
    char msg[INPUTSIZ];
};

// how to receive a string
void get_hello(int socket)
{
    char hello_string[32];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));	
    readn(socket, (unsigned char *) hello_string, k);

    printf("Hello String: %s\n", hello_string);
    printf("Received: %zu bytes\n\n", k);
} // end get_hello()

int get_input(char *input, int length)
{
	int ch;

	// Eat anything left in input buffer.
	while ((ch = getchar()) != '\n' && ch != EOF);
	fgets(input, length, stdin);

	// Make sure string ends in NULL
	int len = strlen(input);
	if ((len > 0) && (input[len - 1] == '\n')) {
        input[len - 1] = '\0';
	}
	
	return len;
}

void handle_message(int socket)
{
	char input[INPUTSIZ];
	printf("Enter message: ");

	// Empty input buffer
	int len = get_input(input, INPUTSIZ);

	printf("Sending: %s\n", input);

	// writen(socket, (unsigned char *) &n, sizeof(int));	
	writen(socket, (unsigned char *) input, len);	
}


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

    // ***
    // your own application code will go here and replace what is below... 
    // i.e. your menu etc.


    // get a string from the server
    get_hello(sockfd);

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
    			handle_message(sockfd);
    		break;
    		case 2:
    			printf("Goodbye!\n");
				running = 0;
    		break;
    		default:
    			fprintf(stderr, "%s", "Error: invalid input\n");
    		break;
    	}
    }

    // *** make sure sockets are cleaned up

    close(sockfd);

    exit(EXIT_SUCCESS);
} // end main()
