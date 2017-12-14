#ifndef _CLIENT_H_
#define _CLIENT_H_

#define LISTENQ         5
#define FDSIZE          1000
#define EPOLLEVENTS     100
#define MAXSIZE         1024

int create_and_connect();

void handle_connection(int sockfd);

void add_event(int epollfd, int fd, int state);

void delete_event(int epollfd, int fd, int state);

void modify_event(int epollfd, int fd, int state);

void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char *buf);

void do_read(int epollfd, int fd, int sockfd, char *buf);

void do_write(int epollfd, int fd, int sockfd, char *buf);

#endif
