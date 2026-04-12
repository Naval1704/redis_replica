#include <iostream>
#include "server.h"
#include "config.h"
using namespace std;

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed" << endl;
        return 1;
    }

    Config config;
    config.load_config("redis.conf");

    int port = stoi(config.get_config_value("port"));
    int maxmemory = stoi(config.get_config_value("maxmemory"));
    string aof_enabled = config.get_config_value("appendonly");
    string aof_file = config.get_config_value("appendfilename");

    CommandHandler handler(maxmemory);
    handler.init_aof(aof_file, aof_enabled == "yes");

    Server server;
    server.start(port, handler);

    WSACleanup();
    return 0;
}
