#include "shared.h"

void die(char *message) 
{
	puts(message);
	exit(EXIT_FAILURE);
}

size_t write_socket(int sockfd, unsigned char *buffer, int length)
{
	size_t result = writen(sockfd, buffer, length);
	if (result <= 0)
	{
		close(sockfd);
		die("Error - read socket error");
	}
	return result;
}

size_t read_socket(int sockfd, unsigned char *buffer, int length) 
{
	size_t result = readn(sockfd, (unsigned char *)buffer, length);
	if (result == 0) 
	{
		close(sockfd);
		die("Error - connection lost");
	}
	else if (result < 0)
	{
		close(sockfd);
		die("Error - read socket error");
	}
	return result;
}
