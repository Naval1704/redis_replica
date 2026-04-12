#include "pubsub.h"
#include "resp_parser.h"
#include <string>

void PubSub::subscribe(SOCKET client, const std::string& channel) {
    channels_[channel].insert(client);
    client_channels_[client].insert(channel);

    // send subscribe confirmation: *3\r\n$9\r\nsubscribe\r\n$<len>\r\n<channel>\r\n:<count>\r\n
    RESPParser resp;
    int count = (int)client_channels_[client].size();
    std::string msg = "*3\r\n"
        + resp.serialize_bulk_string("subscribe")
        + resp.serialize_bulk_string(channel)
        + resp.serialize_integer(count);
    send(client, msg.c_str(), (int)msg.size(), 0);
}

void PubSub::unsubscribe(SOCKET client, const std::string& channel) {
    channels_[channel].erase(client);
    if (channels_[channel].empty()) channels_.erase(channel);
    client_channels_[client].erase(channel);

    RESPParser resp;
    int count = (int)client_channels_[client].size();
    std::string msg = "*3\r\n"
        + resp.serialize_bulk_string("unsubscribe")
        + resp.serialize_bulk_string(channel)
        + resp.serialize_integer(count);
    send(client, msg.c_str(), (int)msg.size(), 0);
}

void PubSub::unsubscribe_all(SOCKET client) {
    auto it = client_channels_.find(client);
    if (it == client_channels_.end()) return;

    // copy the set since we modify it during iteration
    auto channels = it->second;
    for (const auto& ch : channels) {
        unsubscribe(client, ch);
    }
    client_channels_.erase(client);
}

int PubSub::publish(const std::string& channel, const std::string& message) {
    auto it = channels_.find(channel);
    if (it == channels_.end()) return 0;

    RESPParser resp;
    std::string msg = "*3\r\n"
        + resp.serialize_bulk_string("message")
        + resp.serialize_bulk_string(channel)
        + resp.serialize_bulk_string(message);

    int count = 0;
    for (SOCKET client : it->second) {
        send(client, msg.c_str(), (int)msg.size(), 0);
        count++;
    }
    return count;
}

int PubSub::subscription_count(SOCKET client) const {
    auto it = client_channels_.find(client);
    if (it == client_channels_.end()) return 0;
    return (int)it->second.size();
}
