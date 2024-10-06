//
// Created by ener9 on 18/09/2024.
//

#ifndef COURSEWORK_SERVER_SOCKET_H
#define COURSEWORK_SERVER_SOCKET_H

#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mutex>
#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "Ws2_32.lib")


class Socket {
public:
    static Socket& getInstance();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // Server methods
    bool start(int port);
    void stop();

private:
    bool run = false;
    // Private constructor and destructor for singleton
    Socket();
    ~Socket();

    // Helper methods
    static void handleClient(SSL* ssl, SOCKET clientSocket);
    void SocketConnectionLoop();

    // Member variables
    SOCKET serverSocket;
    bool running;
    sockaddr_in serverAddr{};

    SSL_CTX* ctx;

};


#endif //COURSEWORK_SERVER_SOCKET_H
