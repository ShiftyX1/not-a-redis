#include "server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

Server::Server(const Config& config)
    : address(config.get_address()), port(config.get_port()), sockfd(-1) {
}

Server::~Server() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

void Server::setup() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw std::runtime_error("Error creating socket");
    }

    int val = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) != 1) {
        throw std::runtime_error("Error parsing address: " + address);
    }

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        throw std::runtime_error("Error binding to " + address + ":" + std::to_string(port));
    }

    if (listen(sockfd, SOMAXCONN) < 0) {
        throw std::runtime_error("Error listening for connections");
    }

    std::cout << "Server started on " << address << ":" << port << "\n";
}

void Server::handle_connection(int connectionfd) {
    char rbuf[64] = {};
    ssize_t n = read(connectionfd, rbuf, sizeof(rbuf) - 1);
    if (n < 0) {
        std::cerr << "Error reading client packet\n";
        return;
    }
    std::cout << "Client says: " << std::string(rbuf, n) << "\n";
    ssize_t written = write(connectionfd, "Hello, client!\n", 14);
    if (written < 0) {
        std::cerr << "Error writing to client\n";
    }
}

void Server::run() {
    setup();

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connectionfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
        if (connectionfd < 0) {
            continue;
        }
        handle_connection(connectionfd);
        close(connectionfd);
    }
}
