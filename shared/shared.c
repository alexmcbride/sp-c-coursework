/*
 * Author : Alex McBride
 * Student ID: S1715224
 * Date: 05/12/2017
 Code shared between both client and server for the C coursework.
 */
#include "shared.h"

/*
 * Outputs the error and kills the process.
 */
void die(char *message)
{
	printf("%s\n", message);

	// Check if there's an error set.
	if (errno > 0)
	{
		printf("%s\n", strerror(errno));
	}

	// Bye!
	exit(EXIT_FAILURE);
}

/*
 * Wrapper round writen to add some error handling.
 */
size_t write_socket(int sockfd, unsigned char *buffer, int length)
{
	int result = writen(sockfd, buffer, length);
	if (result <= 0)
	{
		close(sockfd);
		die("Error - write socket");
	}
	return result;
}

/*
 * Wrapper round readn to add some error handling.
 */
size_t read_socket(int sockfd, unsigned char *buffer, int length)
{
	int result = readn(sockfd, (unsigned char *)buffer, length);
	if (result == 0)
	{
		close(sockfd);
		die("Error - connection closed");
	}
	else if (result < 0)
	{
		close(sockfd);
		die("Error - read socket");
	}
	return result;
}

/*
 * Sends a string on the socket, prefixed with its length.
 */
void send_message(int socket, char *message)
{
    size_t length = strlen(message) + 1; // Add one to account for NULL terminator
    write_socket(socket, (unsigned char *) &length, sizeof(size_t));
    write_socket(socket, (unsigned char *) message, length);
}

/*
 * Gets a string from the socket, prefixed with its length.
 */
void get_message(int sockfd, char *message)
{
    size_t length;
    read_socket(sockfd, (unsigned char *) &length, sizeof(size_t));
    read_socket(sockfd, (unsigned char *) message, length);
}
