//
// Created by solitude on 18-6-8.
//

#ifndef MINLOADBLANCE_LOG_HPP
#define MINLOADBLANCE_LOG_HPP
#include <syslog.h>
#include <cstdarg>

void set_loglevel(int log_level = LOG_DEBUG);
void log(int log_level, const char *file_name, int line_num, const char* format, ...);


#endif //MINLOADBLANCE_LOG_HPP
