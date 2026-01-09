cmake_minimum_required(VERSION 3.10)

project(MyOwnRedis)


set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/utils.cpp
    src/config.cpp
    src/server.cpp
)

add_executable(my_own_redis ${SOURCES})

