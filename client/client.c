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
#include <sys/stat.h>
#include "../shared/shared.h"
#include "../shared/hexdump.h"

// Constants
#define HOST_ADDRESS "127.0.0.1"

// Function declarations
void handle_server(int sockfd);
void send_request(int sockfd, int request_code);
void get_and_display_message(int sockfd);
void get_uname(int sockfd);
void get_file_list(int sockdf);
int request_file_transfer(int sockfd, char *filename);
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

    // Get input from user.
    char input_str[INPUTSIZ] = {0};
    
    while (fgets(input_str, sizeof(input_str), stdin) != NULL){
        // Convert input to int.
        int input = 0;
        if (sscanf(input_str, "%d", &input) != EOF)
        {
            return input;
        }
    }

    return -1;
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
                if (request_file_transfer(sockfd, filename))
                {
                    get_file_transfer(sockfd, filename);
                }
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
        printf(">> %s\n", filename);
    }
}

int request_file_transfer(int sockfd, char *filename)
{
    // Get filename from user
    printf("Enter filename: ");
    scanf("%255s", filename);
    filename[strcspn(filename, "\n")] = 0;

    // Check file does not already exist on the client
    struct stat buffer;
    if (stat (filename, &buffer) == 0)
    {
        printf("Error - file '%s' already exists\n", filename);
        return 0;
    }

    // Request server sends this file
    send_request(sockfd, REQUEST_FILE_TRANSFER);
    send_message(sockfd, filename);

    return 1;
}

void get_file_transfer(int sockfd, char *filename)
{
    int file_status;
    char error[INPUTSIZ] = {0};
    char buffer[BUFSIZ] = {0};
    int total_bytes;
    int bytes_read;
    FILE *file;

    // First get file status, either OK or ERROR
    read_socket(sockfd, (unsigned char *) &file_status, sizeof(int));

    switch (file_status)
    {
        case FILE_ERROR:
            // Get error message.
            get_message(sockfd, error);
            printf(">> Error '%s' - %s\n", filename, error);
        break;
        case FILE_OK:
            // Get total size of file to send
            read_socket(sockfd, (unsigned char *)&total_bytes, sizeof(int));

            // Open file for writing on this side
            file = fopen(filename, "w");
            if (file == NULL)
            {
                die("Error - failed to open file");
            }

            // Keep looping until read all bytes
            int bytes_remaining = total_bytes;
            while (bytes_remaining > 0)
            {
                // Read bytes from socket.
                bytes_read = recv(sockfd, buffer, BUFSIZ, 0);
                if (bytes_read > 0)
                {
                    // Write bytes to local file
                    fwrite(buffer, sizeof(char), bytes_read, file);
                    bytes_remaining -= bytes_read;
                    // float percentage = (bytes_remaining / total_bytes) * 100.0;
                    printf(">> Transfered %d of %d bytes\n", total_bytes - bytes_remaining, total_bytes);
                }
                else
                {
                    break;
                }
            }

            // Cleanup
            fclose(file);
            printf(">> File transfer of '%s' complete!\n", filename);
        break;
        default:
            puts("Error - unknown response");
        break;
    }
}
