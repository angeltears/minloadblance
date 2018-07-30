//
// Created by onter on 18-7-21.
//

#ifndef MINLOADBLANCE_FDWAPPER_HPP
#define MINLOADBLANCE_FDWAPPER_HPP

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

enum class RET_CODE{OK = 0, NOTHING = 1, IOERR = -1,CLOSED = -2, BUFFER_FULL = -3, BUFFER_EMPTY = -4, TRY_AGAIN};
enum class OP_TYPE {READ = 0, WAITE, ERROR};
int setnonblocking( int fd );
void add_read_fd( int epollfd, int fd );
void add_write_fd( int epollfd, int fd );
void removefd( int epollfd, int fd );
void closefd( int epollfd, int fd );
void modfd( int epollfd, int fd, int ev );


int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void add_read_fd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void add_write_fd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLOUT | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void closefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void removefd(int epollfd, int fd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
}

void modfd(int epollfd, int fd, int ev)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLET;
    epoll_ctl(epollfd , EPOLL_CTL_MOD, fd, &event);
}
#endif //MINLOADBLANCE_FDWAPPER_HPP
