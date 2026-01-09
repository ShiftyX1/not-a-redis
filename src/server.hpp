#pragma once

#include <string>
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

    void handle_connection(int connectionfd);
    void setup();
};
