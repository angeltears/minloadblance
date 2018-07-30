//
// Created by solitude on 18-6-9.
//

#ifndef MINLOADBLANCE_CONN_HPP
#define MINLOADBLANCE_CONN_HPP


#include <arpa/inet.h>
#include "fdwapper.h"
class conn
{
public:
    conn();
    ~conn();
    void init_clt(int sockfd, const sockaddr_in& client_addr);
    void init_srv(int sockfd, const sockaddr_in& server_addr);
    void reset();
    RET_CODE read_clt();
    RET_CODE write_clt();
    RET_CODE read_srv();
    RET_CODE write_srv();

public:
    static const int BUF_SIZE = 2048;
    char* m_clt_buf;
    /// 已读的cli的bufsize
    int m_clt_read_idx;
    /// 已写的cli的bufsize
    int m_clt_write_idx;
    sockaddr_in m_clt_address;
    int m_cltfd;

    char* m_srv_buf;
    /// 已读的ser的bufsize
    int m_srv_read_idx;
    /// 已写的ser的bufsize
    int m_srv_write_idx;
    sockaddr_in m_srv_address;
    int m_srvfd;

    bool m_srv_closed;
};


conn::conn()
{
    m_srvfd = -1;
    m_clt_buf = new char[BUF_SIZE];
    if (!m_clt_buf)
    {
        throw std::exception();
    }
    m_srv_buf = new char[BUF_SIZE];
    if (!m_srv_buf)
    {
        throw std::exception();
    }
    reset();
}

conn::~conn()
{
    delete[] m_clt_buf;
    delete[] m_srv_buf;
};

/// 将客户端和客户端的地址存入
void conn::init_clt( int sockfd, const sockaddr_in& client_addr )
{
    m_cltfd = sockfd;
    m_clt_address = client_addr;
}
/// 将被代理的服务器地址 和 监听套接字初始话
void conn::init_srv( int sockfd, const sockaddr_in& server_addr )
{
    m_srvfd = sockfd;
    m_srv_address = server_addr;
}
void conn::reset()
{
    m_clt_read_idx = 0;
    m_clt_write_idx = 0;
    m_srv_read_idx = 0;
    m_srv_write_idx = 0;
    m_srv_closed = false;
    m_cltfd = -1;
    memset( m_clt_buf, '\0', BUF_SIZE );
    memset( m_srv_buf, '\0', BUF_SIZE );
}
/// 从客户端读取数据，并写入客户端Buf中
RET_CODE conn::read_clt()
{
    int bytes_read = 0;
    while (true)
    {
        if (m_clt_read_idx >= BUF_SIZE)
        {
            log(LOG_ERR, __FILE__, __LINE__, "%s", "the client read buff is full,let server write");
            return RET_CODE::BUFFER_FULL;
        }

        bytes_read = recv(m_cltfd, m_clt_buf + m_clt_read_idx, BUF_SIZE -m_clt_read_idx, 0);
        if (bytes_read == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return RET_CODE::IOERR;
        }
        else if (bytes_read == 0)
        {
            return RET_CODE::CLOSED;
        }

        m_clt_read_idx += bytes_read;
    }
    return  ( ( m_clt_read_idx - m_clt_write_idx ) > 0 ) ? RET_CODE::OK: RET_CODE::NOTHING;
}
/// 从服务端读取数据，并存入服务端Buf中
RET_CODE conn::read_srv()
{
    int bytes_read = 0;
    while (true)
    {
        if (m_srv_read_idx >= BUF_SIZE)
        {
            log(LOG_ERR, __FILE__, __LINE__, "%s", "the server read buff is full,let server write");
            return RET_CODE::BUFFER_FULL;
        }

        bytes_read = recv(m_srvfd, m_srv_buf + m_srv_read_idx, BUF_SIZE -m_srv_read_idx, 0);
        if (bytes_read == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return RET_CODE::IOERR;
        }
        else if (bytes_read == 0)
        {
            log( LOG_ERR, __FILE__, __LINE__, "%s", "the server should not close the persist connection" );
            return RET_CODE::CLOSED;
        }

        m_srv_read_idx += bytes_read;
    }
    return  ( ( m_srv_read_idx - m_srv_write_idx ) > 0 ) ? RET_CODE::OK: RET_CODE::NOTHING;
}

/// 向服务端写数据，从客户端的buf写
RET_CODE conn::write_srv()
{
    int bytes_write = 0;
    while (true)
    {
        if (m_clt_read_idx <= m_clt_write_idx )
        {
            m_clt_read_idx = 0;
            m_clt_write_idx = 0;
            return RET_CODE::BUFFER_EMPTY;
        }

        bytes_write = send(m_srvfd, m_clt_buf + m_clt_write_idx, m_clt_read_idx - m_clt_write_idx, 0);
        if (bytes_write == -1)
        {
            if( errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return RET_CODE ::TRY_AGAIN;
            }
            log(LOG_ERR, __FILE__, __LINE__, "write server socket failed, %s", strerror(errno));
            return RET_CODE::IOERR;
        }
        else if (bytes_write == 0)
        {
            return RET_CODE::CLOSED;
        }
        m_clt_write_idx += bytes_write;
    }
}
/// 向服务端写数据，从服务端的buf写
RET_CODE conn::write_clt()
{
    int bytes_write = 0;
    while (true)
    {
        if (m_srv_read_idx <= m_srv_write_idx)
        {
            m_srv_read_idx = 0;
            m_srv_write_idx = 0;
            return RET_CODE::BUFFER_EMPTY;
        }

        bytes_write = send(m_cltfd, m_srv_buf + m_srv_write_idx, m_srv_read_idx - m_srv_write_idx, 0);
        if (bytes_write == -1)
        {
            if( errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return RET_CODE ::TRY_AGAIN;
            }
            log(LOG_ERR, __FILE__, __LINE__, "write server socket failed, %s", strerror(errno));
            return RET_CODE::IOERR;
        }
        else if (bytes_write == 0)
        {
            return RET_CODE::CLOSED;
        }
        m_srv_write_idx += bytes_write;
    }
}
#endif //MINLOADBLANCE_CONN_HPP
