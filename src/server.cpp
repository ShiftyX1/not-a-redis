#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <iostream>

#include "server.hpp"
#include "net_util.hpp"
#include "constants.hpp"
#include "utils.hpp"


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

int32_t Server::handle_connection(int connectionfd) {
    std::cout << "Waiting for data from " << connectionfd << "\n";

    uint32_t len_net = 0;
    errno = 0;
    if (read_full(connectionfd, &len_net, sizeof(len_net)) != 0) {
        print_error(errno == 0 ? "EOF from client" : "Error reading length");
        return -1;
    }

    uint32_t len = ntohl(len_net);

    if (len > k_max_message_size) {
        print_error("Message length is too long");
        std::cerr << len << " > " << k_max_message_size << "\n";
        return -1;
    }

    std::string message(len, '\0');
    if (len > 0) {
        if (read_full(connectionfd, message.data(), len) != 0) {
            print_error("Error reading payload");
            return -1;
        }
    }

    std::cout << "Client says: " << message << "\n";

    const std::string reply = "world";
    uint32_t reply_len_net = htonl(static_cast<uint32_t>(reply.size()));

    if (write_all(connectionfd, &reply_len_net, sizeof(reply_len_net)) != 0) return -1;
    if (write_all(connectionfd, reply.data(), reply.size()) != 0) return -1;

    return 0;
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
        std::cout << "Accepted connection from " << connectionfd << "\n";
        while (true) {
            int32_t err = handle_connection(connectionfd);
            if (err) {
                break;
            }
        }
        std::cout << "Closing connection from " << connectionfd << "\n";
        close(connectionfd);
    }
}
