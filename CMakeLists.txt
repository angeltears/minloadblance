cmake_minimum_required(VERSION 3.5.1)
project(minloadblance)

set(CMAKE_CXX_STANDARD 11)



add_executable(minloadblance
        main.cpp
        log.hpp log.cpp
        mgr.h
        processpool.h
        conn.h
        xml/tinyxml2.h
        xml/tinyxml2.cpp loadxml.hpp fdwapper.h)
