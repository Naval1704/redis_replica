#pragma once
#define FD_SETSIZE 1024
#include <WinSock2.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class PubSub {
    // channel -> set of subscribed client sockets
    std::unordered_map<std::string, std::unordered_set<SOCKET>> channels_;
    // client -> set of channels they're subscribed to
    std::unordered_map<SOCKET, std::unordered_set<std::string>> client_channels_;

public:
    void subscribe(SOCKET client, const std::string& channel);
    void unsubscribe(SOCKET client, const std::string& channel);
    void unsubscribe_all(SOCKET client);
    int publish(const std::string& channel, const std::string& message);
    int subscription_count(SOCKET client) const;
};
