#pragma once
#define FD_SETSIZE 1024
#include <WinSock2.h>
#include <string>
#include <vector>
#include "store.h"
#include "expiry_manager.h"
#include "pubsub.h"
#include "aof.h"
#include "resp_parser.h"

class CommandHandler {
    Store store_;
    ExpiryManager expiry_;
    PubSub pubsub_;
    AOF aof_;
    RESPParser resp_;

    void cleanup_expired();

public:
    CommandHandler(int max_keys);
    void init_aof(const std::string& filepath, bool enabled);
    std::string handle(SOCKET client, const std::vector<std::string>& command);
};
