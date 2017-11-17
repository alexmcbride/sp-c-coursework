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

typedef struct {
    int id_number;
    int age;
    float salary;
} employee;

// how to send and receive structs
void send_and_get_employee(int socket, employee *e)  
{
    size_t payload_length = sizeof(employee);

    // send the original struct
    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));	
    writen(socket, (unsigned char *) e, payload_length);	 		

    // get back the altered struct
    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));	   
    readn(socket, (unsigned char *) e, payload_length);

    // print out details of received & altered struct
    printf("Age is %d\n", e->age);
    printf("id is %d\n", e->id_number);
    printf("Salary is %6.2f\n", e->salary);    
} // end send_and_get_employee()

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
    serv_addr.sin_port = htons(50001);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // try to connect...
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  {
	perror("Error - connect failed");
	exit(1);
    } else
       printf("Connected to server...\n");

    // ***
    // your own application code will go here and replace what is below... 
    // i.e. your menu etc.


    // get a string from the server
    get_hello(sockfd);

    // send and receive a changed struct to/from the server
    employee *employee1;		
    employee1 = (employee *) malloc(sizeof(employee));

    // arbitrary values
    employee1->age = 23;
    employee1->id_number = 3;
    employee1->salary = 13000.21;

    int i;
    for (i = 0; i < 5; i++) {
         printf("(Counter: %d)\n", i);
	 send_and_get_employee(sockfd, employee1);
         printf("\n");
    }

    free(employee1);

    // *** make sure sockets are cleaned up

    close(sockfd);

    exit(EXIT_SUCCESS);
} // end main()
