cmake_minimum_required(VERSION 3.9)
project(minloadblance)

set(CMAKE_CXX_STANDARD 11)



add_executable(minloadblance
        main.cpp
        log.hpp log.cpp
        mgr.hpp
        processpool.h
        conn.hpp
        xml/tinyxml2.h
        xml/tinyxml2.cpp loadxml.hpp)