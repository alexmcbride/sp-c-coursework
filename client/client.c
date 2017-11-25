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
#include <sys/utsname.h>
#include <limits.h>
#include "../shared/rdwrn.h"
#include "../shared/shared.h"

// Constants
#define HOST_ADDRESS "127.0.0.1"

// Function declarations
void handle_server(int sockfd);
void send_request(int sockfd, int request_code);
void get_and_display_message(int sockfd);
void get_uname(int sockfd);
void get_file_list(int sockdf);
void request_file_transfer(int sockfd, char *filename);
void get_file_transfer(int sockfd, char *filename);

// Functions
int main(void)
{
    // *** this code down to the next "// ***" does not need to be changed except the port number
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        die("Error - could not create socket");
    }

    serv_addr.sin_family = AF_INET;

    // IP address and port of server we want to connect to
    serv_addr.sin_port = htons(PORT_NUMBER);
    serv_addr.sin_addr.s_addr = inet_addr(HOST_ADDRESS);

    // try to connect...
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
    {
        die("Error - connect failed\n");
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
    printf("3. Get uname info\n");
    printf("4. Get file list\n");
    printf("5. Get file transfer\n");
    printf("6. Quit\n");
    printf("Choose option: ");

    int input;
    scanf("%d", &input);

    return input;
}

void handle_server(int sockfd)
{
    char filename[INPUTSIZ];    

    // get welcome message from the server
    get_and_display_message(sockfd);

    // show menu and have user select option
    int running = 1;
    while (running)
    {
        int input = show_menu();
        switch (input) {
            case REQUEST_STUDENT_ID:
                send_request(sockfd, REQUEST_STUDENT_ID);
                get_and_display_message(sockfd);
            break;
            case REQUEST_TIME:
                send_request(sockfd, REQUEST_TIME);
                get_and_display_message(sockfd);
            break;
            case REQUEST_UNAME:
                send_request(sockfd, REQUEST_UNAME);
                get_uname(sockfd);
            break;
            case REQUEST_FILE_LIST:
                send_request(sockfd, REQUEST_FILE_LIST);
                get_file_list(sockfd);
            break;
            case REQUEST_FILE_TRANSFER:
                request_file_transfer(sockfd, filename);
                get_file_transfer(sockfd, filename);
            break;
            case REQUEST_QUIT:
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

void get_and_display_message(int sockfd)
{
    char message[INPUTSIZ];

    get_message(sockfd, message);

    printf(">> %s\n", message);
}

void get_uname(int sockfd)
{
    struct utsname uts;
    read_socket(sockfd, (unsigned char *)&uts, sizeof(struct utsname));

    printf(">> Node name:    %s\n", uts.nodename);
    printf(">> System name:  %s\n", uts.sysname);
    printf(">> Release:      %s\n", uts.release);
    printf(">> Version:      %s\n", uts.version);
    printf(">> Machine:      %s\n", uts.machine);
}

void get_file_list(int sockfd)
{
    // Server first sends total number of files
    // Then sends each file name. File name preceded with length, also has null terminator
    int total_files;
    read_socket(sockfd, (unsigned char *) &total_files, sizeof(int));

    printf(">> List of server files (%d):\n", total_files);


    for (int i = 0; i < total_files; i++)
    {
        char filename[NAME_MAX]; // max size of file name
        get_message(sockfd, filename);
        printf(">> %d - %s\n", (i + 1), filename);
    }
}

void request_file_transfer(int sockfd, char *filename)
{
    printf("Enter filename: ");
    scanf("%255s", filename);

    send_request(sockfd, REQUEST_FILE_TRANSFER);
    send_message(sockfd, filename);
}

void get_file_transfer(int sockfd, char *filename)
{
    // first is int saying status of transfer
    // next int saying size of file in bytes
    // next is file data

    int file_status;
    read_socket(sockfd, (unsigned char *) &file_status, sizeof(int));

    switch (file_status)
    {
        case FILE_NOT_FOUND:
            printf("Error - file '%s' not on server\n", filename);
        break;
        case FILE_PERMISSION_ERROR:
            printf("Error - file '%s' cannot be read\n", filename);
        break;
        case FILE_OK:
            printf("Starting transfer of: %s\n", filename);

            // Get size of file
            // transfer
        break;
        default:
            puts("Error - unknown response");
        break;
    }
}
