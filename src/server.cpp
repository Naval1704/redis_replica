#include "server.h"
#include <iostream>
using namespace std;

void Server::start(int port, CommandHandler& handler) {
    handler_ = &handler;

    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if( listen_socket == INVALID_SOCKET ) return;

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if( bind(listen_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR ) {
        cout << "Bind failed with error: " << WSAGetLastError() << endl;
        closesocket(listen_socket);
        return;
    }

    if( listen(listen_socket, SOMAXCONN) == SOCKET_ERROR ) {
        cout << "Listen failed with error: " << WSAGetLastError() << endl;
        closesocket(listen_socket);
        return;
    }

    u_long mode = 1;
    ioctlsocket(listen_socket, FIONBIO, &mode);

    cout << "Server listening on port " << port << endl;

    while( true ) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(listen_socket, &read_fds);
        for (auto& client : clients) {
            FD_SET(client, &read_fds);
        }

        select(0, &read_fds, nullptr, nullptr, nullptr);

        if( FD_ISSET(listen_socket, &read_fds) ) {
            SOCKET client_socket = accept(listen_socket, nullptr, nullptr);
            if( client_socket != INVALID_SOCKET ) {
                u_long mode = 1;
                ioctlsocket(client_socket, FIONBIO, &mode);

                clients.push_back(client_socket);
                cout << "New client connected: " << client_socket << endl;
            }
        }

        for( int i = (int)clients.size()-1; i >= 0; i-- ) {
            if( FD_ISSET(clients[i], &read_fds) ) {
                char buffer[1024];
                int bytes_received = recv(clients[i], buffer, sizeof(buffer), 0);
                if( bytes_received > 0 ) {
                    buffer[bytes_received] = '\0';
                    string data(buffer, bytes_received);

                    // parse the RESP command and handle it
                    vector<string> command = parser_.parse(data);
                    if (!command.empty()) {
                        string response = handler_->handle(clients[i], command);
                        if (!response.empty()) {
                            send(clients[i], response.c_str(), (int)response.size(), 0);
                        }
                    }
                } else if( bytes_received == 0 ) {
                    cout << "Client disconnected: " << clients[i] << endl;
                    closesocket(clients[i]);
                    clients.erase(clients.begin() + i);
                } else {
                    int err = WSAGetLastError();
                    if (err != WSAEWOULDBLOCK) {
                        cout << "recv failed with error: " << err << endl;
                        closesocket(clients[i]);
                        clients.erase(clients.begin() + i);
                    }
                }
            }
        }
    }
}
