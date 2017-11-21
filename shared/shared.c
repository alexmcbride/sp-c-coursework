#include "shared.h"

void die(char *message) 
{
	puts(message);
	exit(EXIT_FAILURE);
}

size_t write_socket(int sockfd, unsigned char *buffer, int length)
{
	int result = writen(sockfd, buffer, length);
	if (result <= 0)
	{
		close(sockfd);

		char error[ERR_MSG_SIZE];
		sprintf(error, "Error - write socket error: %d", result);
		die(error);
	}
	return result;
}

size_t read_socket(int sockfd, unsigned char *buffer, int length) 
{
	int result = readn(sockfd, (unsigned char *)buffer, length);
	if (result == 0) 
	{
		close(sockfd);
		die("Error - connection lost");
	}
	else if (result < 0)
	{
		close(sockfd);

		char error[ERR_MSG_SIZE];
		sprintf(error, "Error - read socket error: %d", result);
		die(error);
	}
	return result;
}
