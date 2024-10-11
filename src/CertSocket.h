//
// Created by 22118626 on 11/10/2024.
//

#ifndef CERTSOCKET_H
#define CERTSOCKET_H

#include "./Socket.h"



class CertSocket : public Socket{
public:
    explicit CertSocket();
    ~CertSocket();
    bool start(int port) override;

private:
    SOCKET certsocket;
    SOCKET serverSocket;
};



#endif //CERTSOCKET_H
