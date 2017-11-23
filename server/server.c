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
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/utsname.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include "../shared/rdwrn.h"
#include "../shared/shared.h"

// Constants
#define STUDENT_ID "S1715224"
#define UPLOAD_DIR "upload/"

// Prototypes
void *client_handler(void *);
void handle_student_id(int connfd);
void handle_server_time(int connfd);
void handle_uname(int connfd);
void handle_file_list(int connfd);
void send_message(int socket, char *msg);
void get_ip_address(char *ip_str);
void store_start_time();
void initialize_signal_handler();
static void signal_handler(int sig, siginfo_t *siginfo, void *context);

// Global variables
static struct timeval start_time;

// Functions
int main(void)
{
    int listenfd = 0, connfd = 0;

    store_start_time();

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
        die("Error - failed to listen");
    }
    // end socket setup

    // Init sigaction to handle SIGINT
    initialize_signal_handler();

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    while (1) {
        printf("Waiting for a client to connect...\n");
        connfd = accept(listenfd, (struct sockaddr *) &client_addr, &socksize);
        printf("Connection accepted...\n");

        pthread_t sniffer_thread;
        // third parameter is a pointer to the thread function, fourth is its actual parameter
        if (pthread_create(&sniffer_thread, NULL, client_handler, (void *) &connfd) < 0) {
            die("Error - could not create thread");
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        printf("Handler assigned\n");
    }

    // never reached...
    // ** should include a signal handler to clean up
    // shutdown(listenfd, SHUT_RDWR);
    // close(listenfd);
    exit(EXIT_SUCCESS);
}

void get_ip_address(char *ip_str)
{
    int fd;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));

    // Get IP addr from struct
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    // Copy result into parameter
    strcpy(ip_str, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
}

void handle_student_id(int connfd)
{
    // Get IP address
    char ip_str[INET_ADDRSTRLEN];
    memset(ip_str, 0, INET_ADDRSTRLEN);
    get_ip_address(ip_str);

    // Concat string
    char str[24];
    sprintf(str, "%s:%s", ip_str, STUDENT_ID);

    // Send to client
    send_message(connfd, str);
}

void handle_server_time(int connfd)
{
    // Get time.
    time_t t;
    if ((t = time(NULL)) == -1) {
        die("Error - could not get time");
    }

    // Convert to local time.
    struct tm *tm;
    if ((tm = localtime(&t)) == NULL) {
        die("Error - could not get localtime");
    }

    // Get time string.
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%c", tm);

    // Send time message to client.
    send_message(connfd, time_str);
}

void handle_uname(int connfd)
{
    struct utsname uts;
    if (uname(&uts) == -1) {
        die("uname error");
    }

    write_socket(connfd, (unsigned char *)&uts, sizeof(struct utsname));
}

// https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file
int filter_dir(const struct dirent *e)
{
    struct stat st;

    stat(e->d_name, &st);
    return !(st.st_mode & S_IFDIR);
}

void handle_file_list(int socket)
{
    // check "upload" directory exists
    // if not create it
    // else get list of files
    // concat str
    // send string to
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    if (stat(UPLOAD_DIR, &st) == -1)
    {
        mkdir(UPLOAD_DIR, 0744);
        printf("Created upload directory\n");
    }

    struct dirent **namelist;
    int n;
    if ((n = scandir(UPLOAD_DIR, &namelist, filter_dir, alphasort)) == -1)
    {
        die("Error - scandir");
    }
    else
    {
        printf("n: %d\n", n);

        // Send total number of files
        writen(socket, (unsigned char *)&n, sizeof(int));

        while (n--)
        {
            // Get length of name string (d_name ends with NULL char).
            int length = strlen(namelist[n]->d_name);
            writen(socket, (unsigned char *) &length, sizeof(int));
            writen(socket, (unsigned char *) namelist[n]->d_name, length);

            printf("%s\n", namelist[n]->d_name);

            free(namelist[n]);
        }



        free(namelist);
    }
}

void *client_handler(void *socket_desc)
{
    int connfd = *(int *) socket_desc;

    // Send welcome message to client.
    send_message(connfd, "Welcome to the server!");

    while (1)
    {
        int request_code;
        int count = readn(connfd, (unsigned char *) &request_code, sizeof(int));

        // Check if client disconnected.
        if (count == 0)
        {
            printf("Error - lost client connection\n");
            break;
        }
        else if (count < 0)
        {
            printf("Error - client read error: %d\n", count);
            break;
        }

        // Handle client requests
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
        }
    }

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    // always clean up sockets gracefully
    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    return 0;
}

void send_message(int socket, char *msg)
{
    size_t length = strlen(msg) + 1; // Add one to account for NULL terminator
    writen(socket, (unsigned char *) &length, sizeof(size_t));
    writen(socket, (unsigned char *) msg, length);
}

void store_start_time()
{
    // Store server start time.
    if (gettimeofday(&start_time, NULL) == -1) {
        perror("gettimeofday error");
        exit(EXIT_FAILURE);
    }
}

void initialize_signal_handler()
{
    struct sigaction act;
    memset(&act, '\0', sizeof(act));

    // this is a pointer to a function
    act.sa_sigaction = &signal_handler;

    // the SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        die("sigaction failed");
    }
}

// signal handler to be called on receipt of (in this case) SIGTERM
static void signal_handler(int sig, siginfo_t *siginfo, void *context)
{
    // get "wall clock" time at end
    struct timeval end_time;
    if (gettimeofday(&end_time, NULL) == -1) {
        perror("gettimeofday error");
        exit(EXIT_FAILURE);
    }

    // in microseconds...
    printf("Total execution time = %f seconds\n",
	   (double) (end_time.tv_usec - start_time.tv_usec) / 1000000 +
	   (double) (end_time.tv_sec - start_time.tv_sec));

    printf("PID: %ld, UID: %ld\n", (long) siginfo->si_pid, (long) siginfo->si_uid);
    exit(EXIT_FAILURE);
}
