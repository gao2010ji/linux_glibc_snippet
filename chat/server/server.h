#ifndef _SERVER_H_
#define _SERVER_H_

#define LISTENQ         5
#define FDSIZE          1000
#define EPOLLEVENTS     100
#define MAXSIZE         1024

int bind_and_listen();

void do_epoll(int listenfd);

void add_event(int epollfd, int fd, int state);

void delete_event(int epollfd, int fd, int state);

void modify_event(int epollfd, int fd, int state);

void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf);

void handle_accept(int epollfd, int listenfd);

void do_read(int epollfd, int fd, char *buf);

void do_write(int epollfd, int fd, char *buf);

#endif
