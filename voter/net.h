#ifndef __NET_H_

int writen(int fd, const char* ptr, int nbytes);
int readn(int fd, char* ptr, int maxlen);
int readline(int fd, char* ptr, int maxlen);
void get_file(int sockfd, char* filename);
int open_socket(const char* hostname, int port);

#define __NET_H_

#endif

