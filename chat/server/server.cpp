#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "server.h"
#include "common.h"

int bind_and_listen()
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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if(bind(socket_fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(struct sockaddr)) != 0)
    {
        printf("bind socket error\n");
        return -1;
    }

    if(listen(socket_fd, LISTENQ) != 0)
    {
        printf("listen socket error\n");
        return -1;
    }

    return socket_fd;
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

void handle_accept(int epollfd, int listenfd)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    int clientfd = accept(listenfd, reinterpret_cast<struct sockaddr *>(&client_addr), &addr_len);
    if(clientfd == -1)
    {
        printf("accept socket error\n");
    }
    else
    {
        printf("accept new client: %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
    }
    add_event(epollfd, clientfd, EPOLLIN);
}

void do_read(int epollfd, int fd, char *buf)
{
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
        printf("receve message: %s\n", buf);
        modify_event(epollfd, fd, EPOLLOUT);
    }
}

void do_write(int epollfd, int fd, char *buf)
{
    int retval = write(fd, buf, strlen(buf));
    if(retval == -1)
    {
        printf("write socket error\n");
        close(fd);
        delete_event(epollfd, fd, EPOLLOUT);
    }
    else
    {
        modify_event(epollfd, fd, EPOLLIN);
    }
    memset(buf, 0, MAXSIZE);
}

void handle_events(int epollfd, struct epoll_event *events, int num, int listenfd, char *buf)
{
    for(int i = 0; i < num; ++i)
    {
        int fd = events[i].data.fd;

        if((fd == listenfd) && (events[i].events & EPOLLIN))
        {
            handle_accept(epollfd, listenfd);
        }
        else if(events[i].events & EPOLLIN)
        {
            do_read(epollfd, fd, buf);
        }
        else if(events[i].events & EPOLLOUT)
        {
            do_write(epollfd, fd, buf);
        }
    }
}

void do_epoll(int listenfd)
{
    int epollfd = epoll_create(FDSIZE);
    if(epollfd == -1)
    {
        printf("epoll create error\n");
        return;
    }
    add_event(epollfd, listenfd, EPOLLIN);

    struct epoll_event events[EPOLLEVENTS];
    char buf[MAXSIZE] = { 0 };
    while(true)
    {
        int retval = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle_events(epollfd, events, retval, listenfd, buf);
    }
    close(epollfd);
}

int main(int argc, char *argv[])
{
    int retval = bind_and_listen();
    if(retval == -1)
    {
        printf("create listen socket error\n");
        return 0;
    }
    do_epoll(retval);
    close(retval);

    return 0;
}
