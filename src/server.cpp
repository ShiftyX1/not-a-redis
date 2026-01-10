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
#include "logger.hpp"


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

    Logger::log_info("Server started on " + address + ":" + std::to_string(port));
}

int32_t Server::handle_connection(int connectionfd) {
    Logger::log_debug("Waiting for data from fd=" + std::to_string(connectionfd));

    uint32_t len_net = 0;
    errno = 0;
    if (read_full(connectionfd, &len_net, sizeof(len_net)) != 0) {
        if (errno == 0) {
            Logger::log_info("Client disconnected (EOF) from fd=" + std::to_string(connectionfd));
        } else {
            Logger::log_error("Error reading length from fd=" + std::to_string(connectionfd) + ": " + std::strerror(errno));
        }
        return -1;
    }

    uint32_t len = ntohl(len_net);

    if (len > k_max_message_size) {
        Logger::log_error("Message length is too long: " + std::to_string(len) + " (max: " + std::to_string(k_max_message_size) + ")");
        return -1;
    }

    std::string message(len, '\0');
    if (len > 0) {
        if (read_full(connectionfd, message.data(), len) != 0) {
            Logger::log_error("Error reading payload from fd=" + std::to_string(connectionfd));
            return -1;
        }
    }

    Logger::log_info("Client (fd=" + std::to_string(connectionfd) + ") says: " + message);

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
            Logger::log_error("accept() error: " + std::string(std::strerror(errno)));
            continue;
        }

        Logger::log_info("Accepted connection from " + std::string(inet_ntoa(client_addr.sin_addr)) + ":" + std::to_string(ntohs(client_addr.sin_port)) + " (fd=" + std::to_string(connectionfd) + ")");

        while (true) {
            int32_t err = handle_connection(connectionfd);
            if (err) {
                break;
            }
        }
        Logger::log_info("Closing connection from " + std::to_string(connectionfd));
        close(connectionfd);
    }
}
