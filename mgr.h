//
// Created by solitude on 18-6-8.
//

#ifndef MINLOADBLANCE_MGR_HPP
#define MINLOADBLANCE_MGR_HPP
#include <map>
#include "conn.h"
using std::map;
using std::pair;
const unsigned int hostname_size = 1024;

class host
{
public:
    char m_hostname[1024];
    int m_port;
    int m_connect;
};


class mgr
{
public:
    mgr(int epollfd, const host& srv);
    ~mgr() = default;
    int conn2srv(const sockaddr_in & address);
    conn* pick_conn(int cltfd);
    void free_conn(conn* connection);
    int get_used_conn_cnt();
    void recycle_conns();
    RET_CODE process(int fd, OP_TYPE type);
private:
    /// epollfd 共用
    static int m_epollfd;
    /// 被代理的服务器地址和监听套接字的pair
    map<int, conn*> m_conns;
    map<int, conn*> m_used;
    map<int, conn*> m_freed;
    /// 每一个mgr 对应一个host信息
    host m_logic_srv;
};


int mgr::m_epollfd= -1;
int mgr::conn2srv(const sockaddr_in &address)
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) != 0)
    {
        close(sockfd);
        return -1;
    }
    return sockfd;
}


/// 创建epollfd, srv即xml文件中的三个配置信息
mgr::mgr( int epollfd, const host& srv ) : m_logic_srv(srv)
{
    m_epollfd = epollfd;
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, srv.m_hostname, &address.sin_addr);
    address.sin_port = htons(srv.m_port);
    log(LOG_INFO, __FILE__, __LINE__, "logcial srv host info: (%s, %d)", srv.m_hostname, srv.m_port);

    for (int i = 0; i < srv.m_connect; ++i)
    {
        sleep(1);
        int sockfd = conn2srv(address);    /// 创建一个socket 并链接到被代理的端口上 协议是inet4
        if (sockfd < 0)
        {
            log(LOG_ERR, __FILE__, __LINE__, "build connection %d failed", i);
        }
        else
        {
            log(LOG_INFO, __FILE__, __LINE__, "build connection %d sucessed", i);
            conn* tmp = NULL;
            try
            {
                tmp = new conn;
            }
            catch(...)
            {
                close(sockfd);
                continue;
            }
            tmp->init_srv(sockfd, address);
            m_conns.insert(pair<int, conn*>(sockfd, tmp));
        }
    }
}

int mgr::get_used_conn_cnt()
{
    return m_used.size();
}

conn* mgr::pick_conn(int cltfd)
{
    if (m_conns.empty())    /// 判断是否有链接
    {
        log( LOG_ERR, __FILE__, __LINE__, "%s", "not enough srv connections to server" );
        return NULL;
    }
    auto iter = m_conns.begin();
    int srvfd = iter->first;
    conn* tmp = iter->second;
    if (!tmp)
    {
        log( LOG_ERR, __FILE__, __LINE__, "%s", "empty server connection object" );
        return NULL;
    }
    /// 有链接的话就从m_conns中移除，加入到m_used中
    m_conns.erase(iter);
    m_used.insert(pair<int, conn*>(cltfd, tmp));       /// clt即客户端
    m_used.insert(pair<int, conn*>(srvfd, tmp));       /// srv即服务端
    /// 同时检测两个链接的fd
    add_read_fd(m_epollfd, cltfd);
    add_read_fd(m_epollfd, srvfd);
    log(LOG_INFO,__FILE__, __LINE__, "bind client sock %d with server sock %d", cltfd, srvfd);
    /// 将被代理服务器信息返回
    return tmp;
}

void mgr::free_conn(conn *connection)
{
    int cltfd = connection->m_cltfd;
    int srvfd = connection->m_srvfd;
    closefd(m_epollfd, cltfd);
    closefd(m_epollfd, srvfd);
    m_used.erase(cltfd);
    m_used.erase(srvfd);
    connection->reset();
    /// 将释放掉的加入到freed中
    m_freed.insert(pair<int, conn*>(srvfd, connection));
}

void mgr::recycle_conns()
{
    /// 判断有没有空闲的链接
    if (m_freed.empty())
    {
        return;
    }
    for (auto iter = m_freed.begin(); iter != m_freed.end(); iter++)
    {
        sleep(1);
        int srvfd = iter->first;
        conn* tmp = iter->second;
        srvfd = conn2srv(tmp->m_srv_address);        /// 如果有就重新建立链接
        if (srvfd < 0)
        {
            log(LOG_ERR, __FILE__, __LINE__, "%s", "fix connection failed");
        }
        else
        {
            log(LOG_INFO, __FILE__, __LINE__, "%s", "fix connection success");
            tmp->init_srv(srvfd, tmp->m_srv_address);
            m_conns.insert(pair<int, conn*>(srvfd, tmp));
        }
    }
    m_freed.clear();    /// 建立完成后清除数组
}

RET_CODE mgr::process(int fd, OP_TYPE type)
{
    conn* connection = m_used[fd];
    if (!connection)
    {
        return RET_CODE::NOTHING;
    }
    if (connection->m_cltfd == fd)         /// 表明是客户端的链接
    {
        int srvfd = connection->m_srvfd;
        switch (type)
        {
            case OP_TYPE::READ:
            {
                RET_CODE res = connection->read_clt();
                switch(res)
                {
                    case RET_CODE::OK:
                    {
                        log(LOG_DEBUG,__FILE__, __LINE__, "content read from clients : %s", connection->m_clt_buf);
                    }
                    case RET_CODE::BUFFER_FULL:
                    {
                        /// 如果从clt读取的buf满了 就将srvfd设置为out
                        modfd(m_epollfd, srvfd, EPOLLOUT);
                        break;
                    }
                    case RET_CODE::IOERR:
                    {}
                    case RET_CODE::CLOSED:
                    {
                        /// 如果链接断开 就释放掉两个conn
                        free_conn(connection);
                        return RET_CODE::CLOSED;
                    }
                    default:
                        break;
                }
                if (connection->m_srv_closed)
                {
                    free_conn(connection);
                    return RET_CODE::CLOSED;
                }
                break;
            }
            case OP_TYPE::WAITE:
            {
                RET_CODE res = connection->write_clt();
                switch (res)
                {
                    case RET_CODE::TRY_AGAIN:
                    {
                        modfd(m_epollfd, fd, EPOLLOUT);
                        break;
                    }
                    case RET_CODE::BUFFER_EMPTY:
                    {
                        /// 如果要写入客户端的buff为空，则将其设为可读
                        modfd(m_epollfd, srvfd, EPOLLIN);
                        modfd(m_epollfd, fd, EPOLLIN);
                        break;
                    }
                    case RET_CODE::IOERR:
                    {
                    }
                    case RET_CODE::CLOSED:
                    {
                        free_conn(connection);
                        return RET_CODE::CLOSED;
                    }
                    default:
                        break;
                }
                if( connection->m_srv_closed )
                {
                    free_conn( connection );
                    return RET_CODE::CLOSED;
                }
                break;
            }
            default:
            {
                log(LOG_ERR,__FILE__, __LINE__, "%s", "other operation not support yet");
                break;
            }
        }
    }
    else if (connection->m_srvfd == fd)
    {
        int cltfd = connection->m_cltfd;
        switch (type)
        {
            case OP_TYPE::READ:
            {
                RET_CODE res = connection->read_srv();
                switch (res)
                {
                    case RET_CODE::OK:
                    {
                        log(LOG_DEBUG, __FILE__, __LINE__, "content read from server: %s", connection->m_srv_buf);
                    }
                    case RET_CODE::BUFFER_FULL:
                    {
                        modfd(m_epollfd, cltfd, EPOLLOUT);
                        break;
                    }
                    case RET_CODE::IOERR:
                    case RET_CODE::CLOSED:
                    {
                        /// 如果服务端读满完了，就应该想客户端写
                        modfd(m_epollfd, cltfd, EPOLLOUT);
                        connection->m_srv_closed = true;
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case OP_TYPE::WAITE:
            {
                RET_CODE res = connection->write_srv();
                switch (res)
                {
                    case RET_CODE::TRY_AGAIN:
                    {
                        modfd(m_epollfd, fd, EPOLLOUT);
                        break;
                    }
                    case RET_CODE::BUFFER_EMPTY:
                    {
                        modfd(m_epollfd, cltfd, EPOLLIN);
                        modfd(m_epollfd, fd, EPOLLIN);
                        break;
                    }
                    case RET_CODE::IOERR:
                    {

                    }
                    case RET_CODE::CLOSED:  /// 发生了错误，写入客户端失败
                    {
                        modfd( m_epollfd, cltfd, EPOLLOUT );
                        connection->m_srv_closed = true;
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            default:
            {
                log( LOG_ERR, __FILE__, __LINE__, "%s", "other operation not support yet" );
                break;
            }
        }
    }
    else
    {
        return RET_CODE::NOTHING;
    }
    return RET_CODE::OK;
}

#endif //MINLOADBLANCE_MGR_HPP
