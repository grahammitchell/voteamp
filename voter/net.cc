#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "globals.h"

/***********************************************************************
	Write "n" bytes to a stream socket.
************************************************************************/
int writen(int fd, const char* ptr, int nbytes)
{
	int nleft, nwritten;

	nleft = nbytes;

	while(nleft > 0) {
		nwritten = write(fd, ptr, nleft);
		if ( nwritten <= 0 ) 
			return nwritten;                          /* error */

		nleft -= nwritten;
		ptr += nwritten;
	}

	return (nbytes - nleft);
}


/***********************************************************************
	Read a line from a descriptor. Store the new line in the buffer.
	Return the number of characters up to, but not including, the null.
************************************************************************/
int readline(int fd, char* ptr, int maxlen)
{
	int  n, rc;
	char c;

	for ( n=1 ; n<maxlen ; n++ ) {
		if ( (rc = read(fd, &c, 1)) == 1 ) {
			*ptr++ = c;
			if ( c == '\n' ) break;
		}
		else if ( rc == 0 ) {
			if ( n == 1 )  return(0);   /* EOF, no data read */
			else break;                 /* EOF, some data was read */
		}
		else return(-1);                /* error */
	}

	*ptr = 0;

	return n;
}


int readn(int fd, char* ptr, int nbytes)
{
	int  n, rc;
	char c;

	for ( n=1 ; n<nbytes ; n++ ) {
		if ( (rc = read(fd, &c, 1)) == 1 ) {
			*ptr++ = c;
			if ( c == '\0' ) break;
		}
		else if ( rc == 0 ) {
			if ( n == 1 )  return(0);   /* EOF, no data read */
			else break;                 /* EOF, some data was read */
		}
		else return(-1);                /* error */
	}

	*ptr = 0;

	return n;
}


/*************************************************************************
	Gets multiple lines from the named socket and puts them in
	the file named by filename.  Collection continues until reception
	of EOF.  The file is opened for writing and closed before returning.
**************************************************************************/
void get_file(int sockfd, char* filename)
{
	FILE *fd;
	int n;
	char line[MAXLINE];

	fd = fopen(filename, "w");
	n = readline(sockfd, line, MAXLINE);
	while ( n != 0 ) {
		fputs(line, fd);
		n = readline(sockfd, line, MAXLINE);
	}
	fclose(fd);
}


/***********************************************************************
	Establishes a connection with a TCP socket.  Returns socket 
	descriptor.
************************************************************************/
int open_socket(const char* SERVER_NAME, int SERV_TCP_PORT)
{
	int                 sockfd;
	struct sockaddr_in  serv_addr;
	struct hostent      *hp;

	hp = gethostbyname(SERVER_NAME);
	bzero((char *) &serv_addr, sizeof(serv_addr));
	bcopy(hp->h_addr, (char *) &serv_addr.sin_addr, hp->h_length);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_TCP_PORT);

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		fprintf(stderr,"voter: can not open stream socket\n");
		exit(1);
	}

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) {
		fprintf(stderr,"voter: can not connect to server\n");
		exit(2);
	}

	return sockfd;
}

