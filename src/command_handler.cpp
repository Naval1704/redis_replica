#include "command_handler.h"
#include <algorithm>
#include <iostream>

CommandHandler::CommandHandler(int max_keys) : store_(max_keys) {}

void CommandHandler::init_aof(const std::string& filepath, bool enabled) {
    aof_.init(filepath, enabled);

    if (enabled) {
        // replay AOF log to restore state
        std::vector<std::vector<std::string>> commands;
        aof_.replay(commands);
        for (const auto& cmd : commands) {
            if (cmd.empty()) continue;
            std::string name = cmd[0];
            std::transform(name.begin(), name.end(), name.begin(), ::toupper);

            if (name == "SET" && cmd.size() >= 3) {
                store_.set(cmd[1], cmd[2]);
                if (cmd.size() >= 5) {
                    std::string opt = cmd[3];
                    std::transform(opt.begin(), opt.end(), opt.begin(), ::toupper);
                    if (opt == "EX") {
                        expiry_.set_expiry(cmd[1], std::stoi(cmd[4]));
                    }
                }
            } else if (name == "DEL") {
                for (size_t i = 1; i < cmd.size(); i++) {
                    store_.del(cmd[i]);
                    expiry_.remove(cmd[i]);
                }
            } else if (name == "EXPIRE" && cmd.size() >= 3) {
                expiry_.set_expiry(cmd[1], std::stoi(cmd[2]));
            }
        }
        std::cout << "AOF replay: " << commands.size() << " commands restored" << std::endl;
    }
}

void CommandHandler::cleanup_expired() {
    std::vector<std::string> expired;
    expiry_.cleanup(expired);
    for (const auto& key : expired) {
        store_.del(key);
    }
}

std::string CommandHandler::handle(SOCKET client, const std::vector<std::string>& command) {
    if (command.empty()) return resp_.serialize_error("empty command");

    // clean up expired keys before handling
    cleanup_expired();

    std::string name = command[0];
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    // PING
    if (name == "PING") {
        return resp_.serialize_simple_string("PONG");
    }

    // SET key value [EX seconds]
    if (name == "SET") {
        if (command.size() < 3) return resp_.serialize_error("wrong number of arguments for 'SET'");
        store_.set(command[1], command[2]);
        aof_.append(command);

        if (command.size() >= 5) {
            std::string opt = command[3];
            std::transform(opt.begin(), opt.end(), opt.begin(), ::toupper);
            if (opt == "EX") {
                int seconds = std::stoi(command[4]);
                expiry_.set_expiry(command[1], seconds);
            }
        }
        return resp_.serialize_simple_string("OK");
    }

    // GET key
    if (name == "GET") {
        if (command.size() < 2) return resp_.serialize_error("wrong number of arguments for 'GET'");
        // check expiry first
        if (expiry_.is_expired(command[1])) {
            store_.del(command[1]);
            expiry_.remove(command[1]);
            return resp_.serialize_null();
        }
        std::string value;
        if (store_.get(command[1], value)) {
            return resp_.serialize_bulk_string(value);
        }
        return resp_.serialize_null();
    }

    // DEL key [key ...]
    if (name == "DEL") {
        if (command.size() < 2) return resp_.serialize_error("wrong number of arguments for 'DEL'");
        int count = 0;
        for (size_t i = 1; i < command.size(); i++) {
            count += store_.del(command[i]);
            expiry_.remove(command[i]);
        }
        aof_.append(command);
        return resp_.serialize_integer(count);
    }

    // EXPIRE key seconds
    if (name == "EXPIRE") {
        if (command.size() < 3) return resp_.serialize_error("wrong number of arguments for 'EXPIRE'");
        std::string value;
        if (!store_.get(command[1], value)) {
            return resp_.serialize_integer(0);
        }
        int seconds = std::stoi(command[2]);
        expiry_.set_expiry(command[1], seconds);
        aof_.append(command);
        return resp_.serialize_integer(1);
    }

    // TTL key
    if (name == "TTL") {
        if (command.size() < 2) return resp_.serialize_error("wrong number of arguments for 'TTL'");
        std::string value;
        if (!store_.get(command[1], value)) {
            return resp_.serialize_integer(-2);  // key not found
        }
        return resp_.serialize_integer(expiry_.ttl(command[1]));
    }

    // SUBSCRIBE channel [channel ...]
    if (name == "SUBSCRIBE") {
        if (command.size() < 2) return resp_.serialize_error("wrong number of arguments for 'SUBSCRIBE'");
        for (size_t i = 1; i < command.size(); i++) {
            pubsub_.subscribe(client, command[i]);
        }
        return "";  // subscribe sends its own responses
    }

    // UNSUBSCRIBE [channel ...]
    if (name == "UNSUBSCRIBE") {
        if (command.size() < 2) {
            pubsub_.unsubscribe_all(client);
        } else {
            for (size_t i = 1; i < command.size(); i++) {
                pubsub_.unsubscribe(client, command[i]);
            }
        }
        return "";  // unsubscribe sends its own responses
    }

    // PUBLISH channel message
    if (name == "PUBLISH") {
        if (command.size() < 3) return resp_.serialize_error("wrong number of arguments for 'PUBLISH'");
        int count = pubsub_.publish(command[1], command[2]);
        return resp_.serialize_integer(count);
    }

    // DBSIZE
    if (name == "DBSIZE") {
        return resp_.serialize_integer(store_.size());
    }

    // FLUSHDB
    if (name == "FLUSHDB") {
        store_.flush();
        expiry_.clear();
        return resp_.serialize_simple_string("OK");
    }

    return resp_.serialize_error("unknown command '" + name + "'");
}
