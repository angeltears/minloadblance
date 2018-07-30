//
// Created by solitude on 18-6-8.
//
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <memory>
#include <cassert>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.hpp"
#include "mgr.h"
#include "processpool.h"
#include "loadxml.hpp"
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
    if (cfg_file[0] == 0)
    {
        log(LOG_ERR, __FILE__, __LINE__, "%s", "please specifiy the config file");
        return 1;
    }
    vector<host> balance_srv;
    vector<host> logical_srv;
    load_xml(cfg_file, &balance_srv, &logical_srv);
    if( (&balance_srv)->size() == 0 || (&logical_srv)->size() == 0 )
    {
        log( LOG_ERR, __FILE__, __LINE__, "%s", "parse config file failed" );
        return 1;
    }
    const char *ip = (&balance_srv)->at(0).m_hostname;
    const int port = (&balance_srv)->at(0).m_port;

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

    processpool< conn, host, mgr >* pool = processpool< conn, host, mgr >::create( listenfd, logical_srv.size() );
    if( pool )
    {
        pool->run( logical_srv );
        delete pool;
    }
    close(listenfd);
    return 0;
}