//
// Created by 22118626 on 11/10/2024.
//

#ifndef CERTSOCKET_H
#define CERTSOCKET_H

#include "./Socket.h"



class CertSocket : public Socket{
public:
    static CertSocket& getInstance();
    CertSocket();
    ~CertSocket() override;

    bool start(int port) override;
    void stop();

private:
    SOCKET certsocket;
    SOCKET serverSocket;
    Socket socketInstance;
    bool running;

    void ConnectionLoop();
    static void sendCertificate(SOCKET clientSocket, const std::string &certFile);

};



#endif //CERTSOCKET_H
