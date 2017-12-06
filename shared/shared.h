/*
 * Author : Alex McBride
 * Student ID: S1715224
 * Date: 05/12/2017
 * Header file for code shared between client and server.
 */

#ifndef _SHARED_H
#define _SHARED_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rdwrn.h"

// General constants
#define PORT_NUMBER 50031
#define INPUTSIZ 256

// Requests that can be sent from the client to the server.
#define REQUEST_STUDENT_ID 1
#define REQUEST_TIME 2
#define REQUEST_UNAME 3
#define REQUEST_FILE_LIST 4
#define REQUEST_FILE_TRANSFER 5
#define REQUEST_QUIT 6

// File status info sent from server to the client.
#define FILE_ERROR 1
#define FILE_OK 2

// Function declarations
void die(char *message);
size_t read_socket(int sockfd, unsigned char *buffer, int length);
size_t write_socket(int sockfd, unsigned char *buffer, int length);
void send_message(int socket, char *message);
void get_message(int socket, char *message);

#endif
