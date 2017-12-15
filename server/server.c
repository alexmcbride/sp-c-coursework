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
#include <sys/utsname.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include "../shared/shared.h"
#include "../shared/hexdump.h"

#define STUDENT_ID "S1715224"
#define UPLOAD_DIR "upload/"

void *client_handler(void *);
void handle_student_id(int connfd);
void handle_server_time(int connfd);
void handle_uname(int connfd);
void handle_file_list(int connfd);
void handle_file_transfer(int connfd);
int filter_dir(const struct dirent *e);
void get_ip_address(char *ip_str);
void store_start_time();
void initialize_signal_handler();
static void signal_handler(int sig, siginfo_t *siginfo, void *context);
void create_upload_directory();
void send_file_error(int sockfd);

static struct timeval start_time;
static int listenfd;

int main(void)
{
    int connfd = 0;

    store_start_time();
    initialize_signal_handler();
    
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t socksize = sizeof(struct sockaddr_in);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER);

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (listen(listenfd, 10) == -1)
    {
        die("Error - failed to listen");
    }

    puts("Waiting for incoming connections...");
    while (1)
    {
        printf("Waiting for a client to connect...\n");
        connfd = accept(listenfd, (struct sockaddr *) &client_addr, &socksize);
        printf("Connection accepted...\n");

        pthread_t sniffer_thread;
        if (pthread_create(&sniffer_thread, NULL, client_handler, (void *) &connfd) < 0)
        {
            die("Error - could not create thread");
        }

        printf("Handler assigned\n");
    }

    exit(EXIT_SUCCESS);
}

void *client_handler(void *socket_desc)
{
    int connfd = *(int *) socket_desc;

    send_message(connfd, "Welcome to the server!");

    while (1)
    {
        int request_code;
        int result = readn(connfd, (unsigned char *) &request_code, sizeof(int));
        if (result == 0)
        {
            break;
        }
        else if (result < 0)
        {
            printf("Error - client read error: %s\n", strerror(errno));
            break;
        }

        switch (request_code)
        {
            case REQUEST_STUDENT_ID:
                handle_student_id(connfd);
            break;
            case REQUEST_TIME:
                handle_server_time(connfd);
            break;
            case REQUEST_UNAME:
                handle_uname(connfd);
            break;
            case REQUEST_FILE_LIST:
                handle_file_list(connfd);
            break;
            case REQUEST_FILE_TRANSFER:
                handle_file_transfer(connfd);
            break;
            default:
                printf("Error - unknown request\n");
            break;
        }
    }

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    return 0;
}

void get_ip_address(char *ip_str)
{
    int fd;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    strcpy(ip_str, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

void handle_student_id(int connfd)
{
    char ip_str[INET_ADDRSTRLEN];
    memset(ip_str, 0, INET_ADDRSTRLEN);
    get_ip_address(ip_str);

    char str[32];
    sprintf(str, "%s:%s", ip_str, STUDENT_ID);

    send_message(connfd, str);
}

void handle_server_time(int connfd)
{
    time_t t;
    if ((t = time(NULL)) == -1)
    {
        die("Error - could not get time");
    }

    struct tm *tm;
    if ((tm = localtime(&t)) == NULL)
    {
        die("Error - could not get localtime");
    }

    char time_str[64];
    strftime(time_str, sizeof(time_str), "%c", tm);

    send_message(connfd, time_str);
}

void handle_uname(int connfd)
{
    struct utsname uts;
    if (uname(&uts) == -1)
    {
        die("Error - uname");
    }
    write_socket(connfd, (unsigned char *)&uts, sizeof(struct utsname));
}

int filter_dir(const struct dirent *e)
{
    return e->d_type == DT_REG;
}

void handle_file_list(int socket)
{
    create_upload_directory();

    struct dirent **namelist;
    int n;
    int status = 0;
    if ((n = scandir(UPLOAD_DIR, &namelist, filter_dir, alphasort)) == -1)
    {
        send_file_error(socket);
    }
    else
    {
        status = FILE_OK;
        write_socket(socket, (unsigned char *)&status, sizeof(int));
        write_socket(socket, (unsigned char *)&n, sizeof(int));

        while (n--)
        {
            send_message(socket, namelist[n]->d_name);

            free(namelist[n]);
        }

        free(namelist);
    }
}

void handle_file_transfer(int sockfd)
{
    int status = 0;
    char filename[NAME_MAX];

    create_upload_directory();

    get_message(sockfd, filename);

    char local_path[NAME_MAX];
    strcpy(local_path, UPLOAD_DIR);
    strcat(local_path, filename);

    int fd = open(local_path, O_RDONLY);
    if (fd == -1)
    {
        send_file_error(sockfd);
        return;
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0)
    {
        send_file_error(sockfd);
        return;
    }

    status = FILE_OK;
    write_socket(sockfd, (unsigned char *)&status, sizeof(int));

    int bytes_remaining = file_stat.st_size;
    write_socket(sockfd, (unsigned char *)&bytes_remaining, sizeof(int));

    off_t offset = 0;
    int bytes_sent = 0;
    while (((bytes_sent = sendfile(sockfd, fd, &offset, BUFSIZ)) > 0) && bytes_remaining > 0) {
        bytes_remaining -= bytes_sent;
    }

    close(fd);
}

void send_file_error(int sockfd)
{
    int status = FILE_ERROR;
    write_socket(sockfd, (unsigned char *)&status, sizeof(int));
    write_socket(sockfd, (unsigned char *)&errno, sizeof(int));
}

void store_start_time()
{
    if (gettimeofday(&start_time, NULL) == -1)
    {
        die("Error - gettimeofday");
    }
}

void initialize_signal_handler()
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = &signal_handler;
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        die("Error - sigaction failed");
    }
}

static void signal_handler(int sig, siginfo_t *siginfo, void *context)
{
    shutdown(listenfd, SHUT_RDWR);
    close(listenfd);

    struct timeval end_time;
    if (gettimeofday(&end_time, NULL) == -1)
    {
        die("Error - gettimeofday");
    }

    printf("\nTotal execution time: %.2f seconds\n",
	   (double) (end_time.tv_usec - start_time.tv_usec) / 1000000 +
	   (double) (end_time.tv_sec - start_time.tv_sec));

    printf("Exiting...\n");

    exit(EXIT_FAILURE);
}

void create_upload_directory()
{
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    if (stat(UPLOAD_DIR, &st) == -1)
    {
        mkdir(UPLOAD_DIR, 0744);
        printf("Created upload directory\n");
    }
}
