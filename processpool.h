//
// Created by solitude on 18-6-8.
//

#ifndef MINLOADBLANCE_PROCESSPOOL_H
#define MINLOADBLANCE_PROCESSPOOL_H

#include <vector>
#include <memory>
#include <mutex>
#include <signal.h>
#include <sys/epoll.h>
#include <wait.h>
#include "fdwapper.h"
using std::mutex;
using std::vector;
using std::shared_ptr;

class process
{
public:
    process() = default;
public:
    int m_busy_ratio;            /// 负载数量
    pid_t m_pid = -1;            /// 保存子进程pid
    int m_pipefd[2];             /// 父进程统一用0，子进程用1
};

template< typename C, typename H, typename M >
class processpool
{
private:
    processpool(int listenfd, int process_number = 8);
public:
    static processpool<C,H,M> * create(int listenfd, int process_number = 8)
    {
        /// dcl 也不安全　可以尝试使用pthread_once
        if (!m_instance)
        {
            if (!m_instance)
            {
                m_instance = new processpool<C, H, M>(listenfd, process_number);
            }
        }
        return m_instance;
    }
    ~processpool() = default;
    void run(const vector<H>& arg);
private:
    void notify_parent_busy_ratio( int pipefd, M* manager );
    int get_most_free_srv();
    void setup_sig_pipe();
    void run_parent();
    void run_child( const vector<H>& arg );
private:
    /// 进程池最大子进程数量
    static const int MAX_PROCESS_NUMBER = 16;
    /// 每个子进程最多能处理的客户数量
    static const int USER_PER_PROCESS = 65536;
    /// epoll最多能处理的事件数
    static const int MAX_EVENT_NUMBER = 10000;
    /// 进程池中的进程总数
    int m_process_number;
    /// 子进程下标
    int m_idx = -1;
    /// 每个进程的epoll内核表
    int m_epollfd;
    /// 监听socket
    int m_listenfd;
    /// 子进程通过m_stop来决定是否停止运行
    bool m_stop = false;
    /// 保存子进程的描述信息
    vector<process> m_sub_process;
    /// 进程池的静态实例
    static processpool< C, H, M >* m_instance;
};

template< typename C, typename H, typename M >
processpool< C, H, M >* processpool< C, H, M >::m_instance = nullptr;

static int EPOLL_WAIT_TIME = 5000;
static int sig_pipefd[2];

///　可重入的事务驱动模型
static void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(sig_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}
/// 设置信号
static void addsig(int sig, void(handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}


template< typename C, typename H, typename M >
processpool<C,H,M>::processpool(int listenfd, int process_number)
    :m_listenfd(listenfd),m_process_number(process_number)
{
    assert( (process_number > 0) && (process_number < MAX_PROCESS_NUMBER) );
    m_sub_process.resize(process_number);

    for (int i = 0; i < process_number; ++i)
    {
        int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_sub_process[i].m_pipefd);
        assert(ret == 0);

        m_sub_process[i].m_pid = fork();
        assert(m_sub_process[i].m_pid >= 0);
        if (m_sub_process[i].m_pid == 0)
        {
            close(m_sub_process[i].m_pipefd[1]);
            m_sub_process[i].m_busy_ratio = 0;
            continue;
        }
        else
        {
            close(m_sub_process[i].m_pipefd[0]);
            m_idx = i;
            break;
        }
    }
}
/// 获得最小负载的进程
template< typename C, typename H, typename M >
int processpool<C,H,M>::get_most_free_srv()
{
    int ratio = m_sub_process[0].m_busy_ratio;
    int idx = 0;
    for (int i = 0; i < m_process_number; ++i)
    {
        if (m_sub_process[i].m_busy_ratio < ratio)
        {
            idx = i;
            ratio = m_sub_process[i].m_busy_ratio;
        }
    }
    return idx;
}

///　统一信号源
template< typename C, typename H, typename M >
void processpool<C,H,M>::setup_sig_pipe()
{
    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
    assert(ret != -1);
    setnonblocking(sig_pipefd[1]);
    add_read_fd(m_epollfd, sig_pipefd[0]);

    addsig(SIGCHLD, sig_handler);
    addsig(SIGTERM, sig_handler);
    addsig(SIGINT, sig_handler);
    addsig(SIGPIPE, SIG_IGN);
}


/// m_idx == -1 是父进程　m_idx != -1　是子进程
/// 下面以此确定运行子进程还是父进程
template< typename C, typename H, typename M >
void processpool<C,H,M>::run(const vector<H>& arg)
{
    if (m_idx != -1)
    {
        run_child(arg);
        return;
    }
    run_parent();
}


/// 通知父进程子进程的负载数量
template< typename C, typename H, typename M >
void processpool<C,H,M>::notify_parent_busy_ratio(int pipefd, M *manager)
{
    int msg = manager->get_used_conn_cnt();
    send(pipefd, (char*)&msg, 1, 0);
}


template< typename C, typename H, typename M >
void processpool<C,H,M>::run_child(const vector<H> &arg)
{
    setup_sig_pipe();     /// 设置统一信号源

    /// 找到对应的管道描述符并监听套接字可读
    int pipefd_read = m_sub_process[m_idx].m_pipefd[1];
    add_read_fd(m_epollfd, pipefd_read);

    epoll_event events[MAX_PROCESS_NUMBER];
    //创建并初始话mgr,链接子进程
    M* manager = new M(m_epollfd, arg[m_idx]);
    assert(manager);

    int number = 0;
    int ret = -1;

    while (!m_stop)
    {
        /// 等待事件发生
        number = epoll_wait(m_epollfd, events, MAX_PROCESS_NUMBER, EPOLL_WAIT_TIME);
        if ((number < 0) && (errno != EINTR))
        {
            log(LOG_ERR, __FILE__, __LINE__, "%s", "epoll failure");
            break;
        }
        /// 没有事件发生的时候，可以对freed重新利用
        if (number == 0)
        {
            manager->recycle_conns();
            continue;
        }

        for(int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            if ((sockfd == pipefd_read) && (events[i].events & EPOLLIN))
            {
                int client = 0;
                /// 从父子进程管道中读取信息，如果读取成功表明有新的客户端上线。
                ret = recv(sockfd, (char*)&client, sizeof(client), 0);
                if ((ret < 0 ) && (errno != EAGAIN ) || ret == 0)
                {
                    continue;
                }
                else
                {
                    struct sockaddr_in client_address;
                    socklen_t  client_adrlength = sizeof(client_address);
                    /// 子进程接受链接
                    int connfd = accept(m_listenfd, (struct sockaddr*)&client_address, &client_adrlength);
                    if (connfd < 0)
                    {
                        log(LOG_ERR, __FILE__, __LINE__, "errno: %s", strerror(errno));
                        continue;
                    }
                    add_read_fd(m_epollfd, connfd);
                    /// 在mgr中注册代理到被代理，代理到客户端的链接，并将被代理端的conn返回
                    C* conn = manager->pick_conn(connfd);
                    if (!conn)
                    {
                        closefd(m_epollfd, connfd);
                        continue;
                    }
                    conn->init_clt(connfd, client_address);
                    notify_parent_busy_ratio(pipefd_read, manager);
                }
            }
             /// 统一信号源，将异步信号处理，变成同步
            else if ((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN))
            {
                char signals[1024];
                ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
                if (ret <= 0)
                {
                    continue;
                } else
                {
                    for (int i = 0; i < ret; ++i)
                    {
                        switch (signals[i])
                        {
                            case SIGCHLD:
                            {
                                pid_t pid;
                                int stat;
                                while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
                                {
                                    continue;
                                }
                                break;
                            }
                            case SIGTERM:
                            case SIGINT: {
                                m_stop = true;
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                    }
                }
            }/// 如果fd可读,调用mgr的方法
            else if( events[i].events & EPOLLIN )
            {
                RET_CODE result = manager->process( sockfd, OP_TYPE::READ );
                switch( result )
                {
                    case RET_CODE::CLOSED:
                    {
                        /// 通知父进程链接已经被释放
                        notify_parent_busy_ratio( pipefd_read, manager );
                        break;
                    }
                    default:
                        break;
                }
            }/// 如果fd可写,调用mgr的方法
            else if( events[i].events & EPOLLOUT )
            {
                RET_CODE result = manager->process( sockfd, OP_TYPE::WAITE );
                switch( result )
                {
                    case RET_CODE::CLOSED:
                    {
                        notify_parent_busy_ratio( pipefd_read, manager );
                        break;
                    }
                    default:
                        break;
                }
            }
            else
            {
                continue;
            }
        }
    }

    close( pipefd_read );
    close( m_epollfd );
}


template< typename C, typename H, typename M >
void processpool<C,H,M>::run_parent()
{
    setup_sig_pipe();    /// 创建统一事件源

    /// 监听所有子进程的管道
    for( int i = 0; i < m_process_number; ++i )
    {
        add_read_fd( m_epollfd, m_sub_process[i].m_pipefd[ 0 ] );
    }

    /// 监听套接字
    add_read_fd( m_epollfd, m_listenfd );

    epoll_event events[ MAX_EVENT_NUMBER ];
    int sub_process_counter = 0;
    int new_conn = 1;
    int number = 0;
    int ret = -1;

    while( ! m_stop )
    {
        number = epoll_wait( m_epollfd, events, MAX_EVENT_NUMBER, EPOLL_WAIT_TIME );
        if ( ( number < 0 ) && ( errno != EINTR ) )
        {
            log( LOG_ERR, __FILE__, __LINE__, "%s", "epoll failure" );
            break;
        }

        for ( int i = 0; i < number; i++ )
        {
            int sockfd = events[i].data.fd;
            ///  表示有新的客户端链接上来
            if( sockfd == m_listenfd )
            {
                /// 获取空闲链接的下标
                int idx = get_most_free_srv();
                /// 给子进程发送消息，表示收到一个客户端的链接
                send( m_sub_process[idx].m_pipefd[0], ( char* )&new_conn, sizeof( new_conn ), 0 );
                log( LOG_INFO, __FILE__, __LINE__, "send request to child %d", idx );
            }
                /// 统一信号源，将异步信号处理，变成同步
            else if( ( sockfd == sig_pipefd[0] ) && ( events[i].events & EPOLLIN ) )
            {
                int sig;
                char signals[1024];
                ret = recv( sig_pipefd[0], signals, sizeof( signals ), 0 );
                if( ret <= 0 )
                {
                    continue;
                }
                else
                {
                    for( int i = 0; i < ret; ++i )
                    {
                        switch( signals[i] )
                        {
                            case SIGCHLD:
                            {
                                pid_t pid;
                                int stat;
                                while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
                                {
                                    for( int i = 0; i < m_process_number; ++i )
                                    {
                                        if( m_sub_process[i].m_pid == pid )
                                        {
                                            log( LOG_INFO, __FILE__, __LINE__, "child %d join", i );
                                            close( m_sub_process[i].m_pipefd[0] );
                                            m_sub_process[i].m_pid = -1;
                                        }
                                    }
                                }
                                m_stop = true;
                                for( int i = 0; i < m_process_number; ++i )
                                {
                                    if( m_sub_process[i].m_pid != -1 )
                                    {
                                        m_stop = false;
                                    }
                                }
                                break;
                            }
                            case SIGTERM:
                            case SIGINT:
                            {
                                log( LOG_INFO, __FILE__, __LINE__, "%s", "kill all the clild now" );
                                for( int i = 0; i < m_process_number; ++i )
                                {
                                    int pid = m_sub_process[i].m_pid;
                                    if( pid != -1 )
                                    {
                                        kill( pid, SIGTERM );
                                    }
                                }
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }
                    }
                }
            }
                /// 如果从子进程的管道可读，就读入信息，并设置其负载数
            else if( events[i].events & EPOLLIN )
            {
                int busy_ratio = 0;
                ret = recv( sockfd, ( char* )&busy_ratio, sizeof( busy_ratio ), 0 );
                if( ( ( ret < 0 ) && ( errno != EAGAIN ) ) || ret == 0 )
                {
                    continue;
                }
                for( int i = 0; i < m_process_number; ++i )
                {
                    if( sockfd == m_sub_process[i].m_pipefd[0] )
                    {
                        m_sub_process[i].m_busy_ratio = busy_ratio;
                        break;
                    }
                }
                continue;
            }
        }
    }

    for( int i = 0; i < m_process_number; ++i )
    {
        closefd( m_epollfd, m_sub_process[i].m_pipefd[ 0 ] );
    }
    close( m_epollfd );
    close( m_listenfd);
}

#endif //MINLOADBLANCE_PROCESSPOOL_H
