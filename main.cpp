//
// Created by solitude on 18-6-8.
//
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <memory>
#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.hpp"
#include "mgr.hpp"
#include "processpool.h"
#include "conn.hpp"
using std::vector;
using std::shared_ptr;
using std::unique_ptr;
static const char *version = "1.0";

static void usage( const char* prog )
{
    log( LOG_INFO, __FILE__, __LINE__,  "usage: %s [-h] [-v] [-f config_file]", prog );
}


int main(int argc, char *argv[])
{
    char cfg_file[1024];
    memset(cfg_file, 0, sizeof(cfg_file));
    int option;
    while((option = getopt(argc, argv, "f:xvh")) != -1)
    {
        switch (option)
        {
            case 'x':
            {
                set_loglevel(LOG_DEBUG);
                break;
            }

            case 'v':
            {
                log( LOG_INFO, __FILE__, __LINE__, "%s %s", basename(argv[0]), version );
                return 0;
            }

            case 'f':
            {
                memcpy( cfg_file, optarg, strlen( optarg ) );
                break;
            }

            case 'h':
            {
                usage( basename(argv[ 0 ]));
                return 0;
            }

            case '?':
            {
                log( LOG_ERR, __FILE__, __LINE__, "un-recognized option %c", option );
                usage( basename( argv[ 0 ] ) );
                return 1;
            }
        }
    }
    if (cfg_file[0] == '/0')
    {
        log(LOG_ERR, __FILE__, __LINE__, "%s", "please specifiy the config file");
        return 1;
    }
    int cfg_fd = open(cfg_file, O_RDONLY);
    if (cfg_fd == -1)
    {
        log( LOG_ERR, __FILE__, __LINE__, "read config file met error: %s", strerror( errno ) );
        return 1;
    }
    struct stat ret_stat;
    if (fstat( cfg_fd, &ret_stat ) < 0)
    {
        log( LOG_ERR, __FILE__, __LINE__, "read config file met error: %s", strerror( errno ) );
        return 1;
    }
    char *buf = new char[ret_stat.st_size + 1];
    memset(buf, 0, ret_stat.st_size + 1);
    ssize_t read_sz = read(cfg_fd, buf, ret_stat.st_size + 1);
    if (read_sz <= 0)
    {
        log( LOG_ERR, __FILE__, __LINE__, "read config file met error: %s", strerror( errno ) );
        return 1;
    }
    /**
     *以下简单的解析config.xml文件
     */
    shared_ptr<vector<host> > balance_srv {new vector<host> };
    shared_ptr<vector<host> > logical_srv {new vector<host> };
    host tmp_host;
    memset(tmp_host.m_hostname, 0, hostname_size);

    bool opentag = false;
    char *tmp_hostname;
    char *tmp_port;
    char *tmp_connect;
    char *head_body = buf;
    char *seg_line;
    char *seg_body;
    char *end_body;
    while (seg_line = strpbrk(head_body, "\n"))
    {
        *seg_line++ = '\0';
        if (strstr( head_body, "<logical_host>" ))
        {
            if (opentag == true)
            {
                log( LOG_ERR, __FILE__, __LINE__, "%s", "parse config file failed" );
                return 1;
            }
            else
            {
                opentag = true;
            }
        }
        else if(strstr(head_body,"</logical_host>" ))
        {
            if( !opentag )
            {
                log( LOG_ERR, __FILE__, __LINE__, "%s", "parse config file failed" );
                return 1;
            }
            logical_srv.get()->push_back(tmp_host);
            memset( tmp_host.m_hostname, '\0', 1024 );
            opentag = false;
        }
        else if((seg_body = strstr(head_body, "<name>")))
        {
            tmp_hostname = seg_body + 6;
            end_body = strstr(tmp_hostname, "</name>");
            if (!end_body)
            {
                log( LOG_ERR, __FILE__, __LINE__, "%s", "parse config file failed" );
                return 1;
            }
            *end_body = '\0';
            memcpy(tmp_host.m_hostname,tmp_hostname, strlen(tmp_hostname));
        }
        else if( (seg_body  = strstr( head_body, "<port>" )) != nullptr)
        {
            tmp_port = seg_body  + 6;
            end_body = strstr( tmp_port, "</port>" );
            if( !end_body )
            {
                log( LOG_ERR, __FILE__, __LINE__, "%s", "parse config file failed" );
                return 1;
            }
            *end_body = '\0';
            tmp_host.m_port = atoi( tmp_port );
        }
        else if( (seg_body = strstr( head_body, "<conns>" )) != nullptr)
        {
            tmp_connect = seg_body+ 7;
            end_body = strstr( tmp_connect, "</conns>" );
            if( !end_body )
            {
                log( LOG_ERR, __FILE__, __LINE__, "%s", "parse config file failed" );
                return 1;
            }
            *end_body = '\0';
            tmp_host.m_connect = atoi( tmp_connect);
        }
        else if((seg_body = strstr(head_body, "Listen")) != nullptr)
        {
            tmp_hostname = seg_body + 7;
            seg_body = strstr(tmp_hostname, ":");
            if (seg_body == nullptr)
            {
                log( LOG_ERR, __FILE__, __LINE__, "%s", "parse config file failed" );
                return 1;
            }
            *seg_body++ = '\0';
            tmp_host.m_port = atoi(seg_body);
            memcpy(tmp_host.m_hostname, tmp_hostname, strlen(tmp_hostname));
            balance_srv.get()->push_back(tmp_host);
            memset( tmp_host.m_hostname, '\0', hostname_size);
        }
        head_body = seg_line;
    }
    if( balance_srv.get()->size() == 0 || logical_srv.get()->size() == 0 )
    {
        log( LOG_ERR, __FILE__, __LINE__, "%s", "parse config file failed" );
        return 1;
    }
    const char *ip = balance_srv.get()->at(0).m_hostname;
    const int port = balance_srv.get()->at(0).m_port;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET,ip, &address.sin_addr);

    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    unique_ptr< processpool<conn,host,mgr> > pool {processpool<conn,host,mgr>::
                                                   create(listenfd, static_cast<unsigned int>(logical_srv.get()->size()))};
    if (pool.get() != nullptr)
    {
        pool.get()->run(logical_srv.get());
    }
    close(listenfd);
    return 0;
}