//
// Created by solitude on 18-6-8.
//
#include <cstdio>
#include <ctime>
#include <cstring>
#include <memory>
#include "log.hpp"
using std::shared_ptr;
static int level = LOG_INFO;
static int BUFFER_SIZE = 4096;
const char *loglevels[] =
        {
                "EMERG", "ALERT", "CRIT","ERR", "WARNING","NOTICE","INFO:",
                "DEBUG:"
        };

void set_loglevel(int log_level)
{
    level = log_level;
}

void log(int log_level, const char *file_name, int line_num, const char* format, ...)
{
    if (log_level > level)
    {
        return;
    }
    time_t tmp = time(nullptr);
    tm* cur_time = localtime(&tmp);
    if (cur_time == nullptr)
    {
        return;
    }

    char arg_buffer[BUFFER_SIZE];
    memset(arg_buffer, 0 , BUFFER_SIZE);
    strftime(arg_buffer,BUFFER_SIZE, "[ %x %X ]", cur_time);
    printf("%s ",arg_buffer);
    printf("%s:%4d ", file_name, line_num);
    printf("%s ",loglevels[log_level - LOG_EMERG]);

    va_list arg_list;
    va_start(arg_list, format);
    memset(arg_buffer, 0, BUFFER_SIZE);
    vsnprintf(arg_buffer, BUFFER_SIZE - 1, format, arg_list);
    printf("%s\n", arg_buffer);
    fflush(stdout);
    va_end(arg_list);
}

