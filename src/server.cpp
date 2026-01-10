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
#include <fcntl.h>
#include <poll.h>

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
    for (auto const& [fd, conn] : fd2conn) {
        delete conn;
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
    
    fd_set_nb(sockfd);

    Logger::log_info("Server started on " + address + ":" + std::to_string(port));
}

void Server::accept_new_connection() {
    struct sockaddr_in client_addr = {};
    socklen_t addrlen = sizeof(client_addr);
    int connectionfd = accept(sockfd, (struct sockaddr *)&client_addr, &addrlen);
    if (connectionfd < 0) {
        Logger::log_error("accept() error: " + std::string(std::strerror(errno)));
        return;
    }

    fd_set_nb(connectionfd);
    
    Connection* conn = new Connection();
    conn->connectionfd = connectionfd;
    conn->state = STATE_REQ;
    
    fd2conn[connectionfd] = conn;
    
    struct pollfd pfd = {};
    pfd.fd = connectionfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    poll_args.push_back(pfd);

    Logger::log_info("Accepted connection from " + std::string(inet_ntoa(client_addr.sin_addr)) + ":" + std::to_string(ntohs(client_addr.sin_port)) + " (fd=" + std::to_string(connectionfd) + ")");
}

void Server::state_req(Connection* conn) {
    while (true) {
        char buf[1024];
        ssize_t rv = read(conn->connectionfd, buf, sizeof(buf));
        if (rv < 0) {
            if (errno == EAGAIN) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            conn->state = STATE_END;
            return;
        }
        if (rv == 0) {
            conn->state = STATE_END;
            return;
        }
        
        conn->incoming.insert(conn->incoming.end(), buf, buf + rv);
    }

    while (conn->state == STATE_REQ) {
        if (conn->incoming.size() < 4) {
            break;
        }
        
        uint32_t len_net;
        memcpy(&len_net, conn->incoming.data(), 4);
        uint32_t len = ntohl(len_net);
        
        if (len > k_max_message_size) {
            Logger::log_error("Message too long");
            conn->state = STATE_END;
            return;
        }
        
        if (conn->incoming.size() < 4 + len) {
            break;
        }
        
        if (parse_and_execute(conn) != 0) {
            conn->state = STATE_END;
            return;
        }

        conn->incoming.erase(conn->incoming.begin(), conn->incoming.begin() + 4 + len);
    }
}

void Server::state_res(Connection* conn) {
    while (!conn->outgoing.empty()) {
        ssize_t rv = write(conn->connectionfd, conn->outgoing.data(), conn->outgoing.size());
        if (rv < 0) {
            if (errno == EAGAIN) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            conn->state = STATE_END;
            return;
        }
        
        conn->outgoing.erase(conn->outgoing.begin(), conn->outgoing.begin() + rv);
    }
    
    if (conn->outgoing.empty()) {
        conn->state = STATE_REQ;
    }
}

int32_t Server::parse_and_execute(Connection* conn) {
    uint32_t len_net;
    memcpy(&len_net, conn->incoming.data(), 4);
    uint32_t len = ntohl(len_net);
    
    std::string message(conn->incoming.begin() + 4, conn->incoming.begin() + 4 + len);
    Logger::log_info("Client (fd=" + std::to_string(conn->connectionfd) + ") says: " + message);
    
    const std::string reply = "world";
    uint32_t reply_len = static_cast<uint32_t>(reply.size());
    uint32_t reply_len_net = htonl(reply_len);
    
    const char* header_ptr = reinterpret_cast<const char*>(&reply_len_net);
    conn->outgoing.insert(conn->outgoing.end(), header_ptr, header_ptr + 4);
    
    conn->outgoing.insert(conn->outgoing.end(), reply.begin(), reply.end());
    
    conn->state = STATE_RES;
    return 0;
}

void Server::connection_io(Connection* conn) {
    if (conn->state == STATE_REQ) {
        state_req(conn);
    } else if (conn->state == STATE_RES) {
        state_res(conn);
    }
}


void Server::run() {
    setup();

    struct pollfd pfd_listen = {sockfd, POLLIN, 0};
    poll_args.push_back(pfd_listen);

    while (true) {
        // Prepare events used in poll
        for (auto& pfd : poll_args) {
            if (pfd.fd == sockfd) {
                pfd.events = POLLIN;
                continue;
            }
            
            auto it = fd2conn.find(pfd.fd);
            if (it != fd2conn.end()) {
                Connection* conn = it->second;
                if (conn->state == STATE_REQ) {
                    pfd.events = POLLIN;
                } else if (conn->state == STATE_RES) {
                    pfd.events = POLLOUT;
                }
            }
        }
        
        int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);
        if (rv < 0) {
            Logger::log_error("poll error");
            break;
        }
        
        for (size_t i = 0; i < poll_args.size(); ++i) {
            if (poll_args[i].revents) {
                int fd = poll_args[i].fd;
                if (fd == sockfd) {
                    accept_new_connection();
                } else {
                    auto it = fd2conn.find(fd);
                    if (it != fd2conn.end()) {
                        connection_io(it->second);
                    }
                }
            }
        }
        
        for (size_t i = 0; i < poll_args.size(); ) {
            int fd = poll_args[i].fd;
            if (fd == sockfd) {
                ++i;
                continue;
            }
            
            auto it = fd2conn.find(fd);
            if (it == fd2conn.end() || it->second->state == STATE_END) {
                if (it != fd2conn.end()) {
                    close(fd);
                    delete it->second;
                    fd2conn.erase(it);
                    Logger::log_info("Closing connection fd=" + std::to_string(fd));
                }
                
                poll_args[i] = poll_args.back();
                poll_args.pop_back();
            } else {
                ++i;
            }
        }
    }
}
