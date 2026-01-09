#pragma once

#include <string>
#include <stdint.h>
#include "config.hpp"

class Server {
public:
    Server(const Config& config);
    ~Server();

    void run();

private:
    int sockfd;
    std::string address;
    int port;

    int32_t handle_connection(int connectionfd);
    void setup();
};
