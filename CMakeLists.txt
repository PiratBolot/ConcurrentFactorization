cmake_minimum_required(VERSION 3.8)
project(ConcurrentFactorization)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Factorizator.cpp Factorizator.h ThreadPool.h BusyThread.h BusyThreadFactorizator.h)
add_executable(ConcurrentFactorization ${SOURCE_FILES})