#include <iostream>
#include <WinSock2.h>
#include "config.h"
using namespace std;

int main() {

    WORD wVersionRequested; // Version of Winsock we want to use
    WSADATA wsaData ; // Structure to hold information about the Windows Sockets implementation
    int err ;

    // MAKEWORD(2,2) creates a 16-bit value that represents the version of Winsock we want to use (2.2 in this case)
    wVersionRequested = MAKEWORD(2,2);

    err = WSAStartup(wVersionRequested, &wsaData);

    // If WSAStartup returns a non-zero value, it indicates an error occurred during initialization
    if( err != 0 ) {
        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }

    // Initialization succeeded
    cout << "The Winsock 2.2 dll was found okay" << endl;

    // read config file 
    Config config;
    config.load_config("redis.conf"); // laoding config file
    cout << config.get_config_value("port") << endl;


    WSACleanup();
    cout << "WSACleanup completed." << endl;

    return 0;
}