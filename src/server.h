#pragma once
#define FD_SETSIZE 1024
#include <winsock2.h>
#include <vector>
#include <string>
#include "command_handler.h"
#include "resp_parser.h"

class Server {
    SOCKET listen_socket;
    std::vector<SOCKET> clients;
    CommandHandler* handler_;
    RESPParser parser_;

public:
    void start(int port, CommandHandler& handler);
};
