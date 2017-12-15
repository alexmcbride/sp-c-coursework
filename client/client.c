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

#define HOST_ADDRESS "127.0.0.1"

void handle_server(int sockfd);
int show_menu();
void send_request(int sockfd, int request_code);
void get_and_display_message(int sockfd);
void get_uname(int sockfd);
void get_file_list(int sockdf);
int request_file_transfer(int sockfd, char *filename);
void get_file_transfer(int sockfd, char *filename);

int main(void)
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        die("Error - could not create socket");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUMBER);
    serv_addr.sin_addr.s_addr = inet_addr(HOST_ADDRESS);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
    {
        die("Error - connect failed\n");
    }
    else
    {
        printf("Connected to server...\n");
    }

    handle_server(sockfd);

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

    char input_str[INPUTSIZ];
    memset(input_str, 0, sizeof(input_str));
    while (fgets(input_str, sizeof(input_str), stdin) != NULL)
    {
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

    get_and_display_message(sockfd);

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
                printf("Exiting...\n");
                running = 0;
            break;
            default:
                printf("Error - invalid input\n");
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
    memset(message, 0, sizeof(message));
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
    int file_status = 0;
    int i;
    read_socket(sockfd, (unsigned char *)&file_status, sizeof(int));

    if (file_status == FILE_ERROR)
    {
        int error_number = 0;
        read_socket(sockfd, (unsigned char *)&error_number, sizeof(int));
        printf(">> Error - failed to read server directory: %s\n", strerror(error_number));
    }
    else if (file_status == FILE_OK)
    {
        int total_files;
        read_socket(sockfd, (unsigned char *) &total_files, sizeof(int));

        printf(">> List of server files (%d):\n", total_files);

        for (i = 0; i < total_files; i++)
        {
            char filename[NAME_MAX];
            get_message(sockfd, filename);
            printf(">> %s\n", filename);
        }
    }
    else
    {
        printf("Error - unknown response\n");
    }
}

int request_file_transfer(int sockfd, char *filename)
{
    printf("Enter filename: ");
    scanf("%255s", filename);
    filename[strcspn(filename, "\n")] = 0;

    struct stat buffer;
    if (stat (filename, &buffer) == 0)
    {
        printf("Error - file '%s' already exists\n", filename);
        return 0;
    }

    send_request(sockfd, REQUEST_FILE_TRANSFER);
    send_message(sockfd, filename);

    return 1;
}

void get_file_transfer(int sockfd, char *filename)
{
    int file_status;
    int error_number;
    char buffer[BUFSIZ];
    int total_bytes;
    int bytes_read;
    FILE *file;

    read_socket(sockfd, (unsigned char *) &file_status, sizeof(int));

    switch (file_status)
    {
        case FILE_ERROR:
            read_socket(sockfd, (unsigned char *)&error_number, sizeof(int));
            printf(">> Error - failed to read server file: %s\n", strerror(error_number));
        break;
        case FILE_OK:
            read_socket(sockfd, (unsigned char *)&total_bytes, sizeof(int));

            file = fopen(filename, "w");
            if (file == NULL)
            {
                printf("Error - failed to open local file: %s\n", strerror(errno));
                return;
            }

            int bytes_remaining = total_bytes;
            while (bytes_remaining > 0)
            {
                bytes_read = recv(sockfd, buffer, BUFSIZ, 0);

                if (bytes_read == 0)
                {
                    break;
                }

                fwrite(buffer, sizeof(char), bytes_read, file);
                bytes_remaining -= bytes_read;

                printf(">> Transfered %d of %d bytes\n", total_bytes - bytes_remaining, total_bytes);
            }

            fclose(file);

            if (bytes_remaining == 0)
            {
                printf(">> Transfer of '%s' complete\n", filename);
            }
            else
            {
                printf(">> Transfer of '%s' interrupted\n", filename);
            }
        break;
        default:
            puts("Error - unknown server response");
        break;
    }
}
