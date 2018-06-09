//
// Created by solitude on 18-6-8.
//

#ifndef MINLOADBLANCE_MGR_HPP
#define MINLOADBLANCE_MGR_HPP

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

};

#endif //MINLOADBLANCE_MGR_HPP
