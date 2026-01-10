cmake_minimum_required(VERSION 3.10)

project(MyOwnRedis)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_REQUIRED_FLAGS -Wall -Wextra -Werror)

set(SOURCES
    src/main.cpp
    src/utils.cpp
    src/config.cpp
    src/server.cpp
    src/net_util.cpp
    src/logger.cpp
)

add_executable(my_own_redis ${SOURCES})

