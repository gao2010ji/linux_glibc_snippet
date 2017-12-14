#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "client.h"
#include "common.h"

int create_and_connect()
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1)
    {
        printf("create socket error\n");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if(connect(socket_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) == - 1)
    {
        printf("connect server: %s error\n", SERVER_IP);
        return -1;
    }

    return socket_fd;
}

void handle_connection(int sockfd)
{
    int epollfd = epoll_create(FDSIZE);
    if(epollfd == -1)
    {
        printf("epoll create error\n");
        return;
    }
    add_event(epollfd, STDIN_FILENO, EPOLLIN);

    struct epoll_event events[EPOLLEVENTS];
    char buf[MAXSIZE] = { 0 };
    memset(buf, 0, MAXSIZE);
    while(true)
    {
        int retval = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, retval, sockfd, buf);
    }
    close(epollfd);
}

void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

void modify_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
}

void do_read(int epollfd, int fd, int sockfd, char *buf)
{
    printf("do_read\n");
    int retval = read(fd, buf, MAXSIZE);
    if(retval == -1)
    {
        printf("read socket error\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }
    else if(retval == 0)
    {
        printf("client close\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLIN);
    }
    else
    {
        if(fd == STDIN_FILENO)
        {
            add_event(epollfd, sockfd, EPOLLOUT);
        }
        else
        {
            delete_event(epollfd, sockfd, EPOLLIN);
            add_event(epollfd, STDOUT_FILENO, EPOLLOUT);
        }
    }
}

void do_write(int epollfd, int fd, int sockfd, char *buf)
{
    printf("do_write\n");
    static int count = 0;
    char temp[100] = { 0 };
    buf[strlen(buf) - 1] = '\0';
    snprintf(temp, sizeof(temp), "%s_%02d\n", buf, count++);
    int retval = write(fd, temp, strlen(temp));
    if(retval == -1)
    {
        printf("write socket error\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLOUT);
    }
    else
    {
        if(fd == STDOUT_FILENO)
        {
            delete_event(epollfd, fd, EPOLLOUT);
        }
        else
        {
            modify_event(epollfd, fd, EPOLLIN);
        }
    }
    memset(buf, 0, MAXSIZE);
}

void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char *buf)
{
    for(int i = 0; i < num; ++i)
    {
        int fd = events[i].data.fd;

        if(events[i].events & EPOLLIN)
        {
            do_read(epollfd, fd, sockfd, buf);
        }
        else if(events[i].events & EPOLLOUT)
        {
            do_write(epollfd, fd, sockfd, buf);
        }
    }
}

int main(int argc, char *argv[])
{
    int retval = create_and_connect();
    if(retval == -1)
    {
        return 0;
    }

    handle_connection(retval);
    close(retval);

    return 0;
}
