#pragma once

#include <string>
#include <stdint.h>
#include "config.hpp"
#include <vector>
#include <map>
#include <poll.h>

enum {
    STATE_REQ = 0,
    STATE_RES = 1,
    STATE_END = 2,
};

struct Connection {
    int connectionfd = -1;
    uint32_t state = STATE_REQ;
    std::vector<uint8_t> incoming;
    std::vector<uint8_t> outgoing;
};

class Server {
public:
    Server(const Config& config);
    ~Server();

    void run();

private:
    int sockfd;
    std::string address;
    int port;

    std::vector<struct pollfd> poll_args;
    std::map<int, Connection*> fd2conn;

    void setup();
    void accept_new_connection();
    void connection_io(Connection* conn);
    void state_req(Connection* conn);
    void state_res(Connection* conn);
    int32_t parse_and_execute(Connection* conn);
};
