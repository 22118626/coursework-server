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

private:
    void sendCertificate(SOCKET clientSocket, const std::string &certFile);

    SOCKET certsocket;
    SOCKET serverSocket;
    Socket socketInstance;


};



#endif //CERTSOCKET_H
