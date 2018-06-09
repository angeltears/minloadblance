//
// Created by solitude on 18-6-8.
//

#ifndef MINLOADBLANCE_PROCESSPOOL_H
#define MINLOADBLANCE_PROCESSPOOL_H

#include <vector>
#include <memory>
#include <mutex>
using std::mutex;
using std::vector;
using std::shared_ptr;

class process
{
public:
    int m_busy_ratio;
    pid_t m_pid = -1;
    int m_pipefd[2];
};

template< typename C, typename H, typename M >
class processpool
{
private:
    processpool(int listenfd, int process_number = 8);

public:
    static processpool<C,H,M> * create(int listenfd, int process_number = 8)
    {
        if (!m_instance)
        {
            mutex tex;
            tex.lock();
            if (!m_instance)
            {
                m_instance = new processpool<C, H, M>(listenfd, process_number);
            }
            tex.unlock();
        }
        return m_instance;
    }
    ~processpool() = default;
private:
    static const int MAX_PROCESS_NUMBER = 16;
    static const int USER_PER_PROCESS = 65536;
    static const int MAX_EVENT_NUMBER = 10000;
    int m_process_number;
    int m_idx = -1;
    int m_epollfd;
    int m_listenfd;
    bool m_stop = false;
    shared_ptr< vector<process> > m_sub_process = nullptr;
    static processpool< C, H, M >* m_instance;
};

template< typename C, typename H, typename M >
processpool< C, H, M >* processpool< C, H, M >::m_instance = nullptr;

template< typename C, typename H, typename M >
processpool<C,H,M>::processpool(int listenfd, int process_number)
:m_listenfd(listenfd),m_process_number(process_number)
{
    assert(process_number > 0 && process_number < MAX_PROCESS_NUMBER);
    m_sub_process.reset(new vector<process>(static_cast<unsigned long>(process_number)));
    assert(m_sub_process.get() != nullptr);

    for(int i = 0; i < process_number; i++)
    {
        int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_sub_process.get()->at(i).m_pipefd);
        assert(ret == 0);
        m_sub_process.get()->at(i).m_pid = fork();
        assert(m_sub_process.get()->at(i).m_pid >= 0);
        if (m_sub_process.get()->at(i).m_pid > 0)
        {
            close(m_sub_process.get()->at(i).m_pipefd[1]);
            m_sub_process.get()->at(i).m_busy_ratio = 0;
            continue;
        }
        else
        {
            close(m_sub_process.get()->at(i).m_pipefd[0]);
            m_idx = i;
            break;
        }

    }
}
#endif //MINLOADBLANCE_PROCESSPOOL_H
