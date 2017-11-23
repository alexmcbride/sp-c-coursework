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

#define UPLOAD_DIR "upload/"

int filter_dir(const struct dirent *e)
{
    struct stat st;

    stat(e->d_name, &st);
    return !(st.st_mode & S_IFDIR);
}

void handle_file_list()
{
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
        printf("Sent total number: %d\n", n);

        while (n--)
        {
            // Get length of name string (d_name ends with NULL char).
            int length = strlen(namelist[n]->d_name);
            printf("Length: %d\n", length);
            printf("Name: %s\n", namelist[n]->d_name);

            free(namelist[n]);
        }

        free(namelist);
    }
}

int main()
{
    handle_file_list();

    exit(EXIT_SUCCESS);
}
