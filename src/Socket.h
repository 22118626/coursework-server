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
#include "./Database.h"

#pragma comment(lib, "Ws2_32.lib")


class Socket {
public:
    static Socket &getInstance();

    /*Socket(const Socket &) = delete;

    Socket &operator=(const Socket &) = delete;*/

    // Server methods
    virtual bool start(int port);

    void stop();

    virtual ~Socket();

protected:


    static void handleClient(SSL *ssl, SOCKET clientSocket);
    void SocketConnectionLoop();

    // Member variables
    SOCKET serverSocket;
    bool running;
    sockaddr_in serverAddr{};

    SSL_CTX *ctx;


    Socket();

private:
    bool run = false;
    static std::shared_ptr<Database> db;
};

#endif //COURSEWORK_SERVER_SOCKET_H
