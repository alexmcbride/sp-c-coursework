#ifndef _SHARED_H
#define _SHARED_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rdwrn.h"

#define PORT_NUMBER 50031
#define ERR_MSG_SIZE 64
#define INPUTSIZ 256

#define REQUEST_STUDENT_ID 1
#define REQUEST_TIME 2
#define REQUEST_UNAME 3
#define REQUEST_FILE_LIST 4
#define REQUEST_FILE_TRANSFER 5
#define REQUEST_QUIT 6

#define FILE_NOT_FOUND 1
#define FILE_PERMISSION_ERROR 2
#define FILE_OK 3

void die(char *message);
size_t read_socket(int sockfd, unsigned char *buffer, int length);
size_t write_socket(int sockfd, unsigned char *buffer, int length);
void send_message(int socket, char *message);
void get_message(int socket, char *message);

#endif
