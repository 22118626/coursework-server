//
// Created by 22118626 on 11/10/2024.
//

#include "CertSocket.h"

CertSocket::CertSocket() :  Socket() {

}
CertSocket::~CertSocket() {
    delete this;
}



bool CertSocket::start(int port) override {
    std::cout << "CertSocket::start" << std::endl;
    certsocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    bind(certsocket, (sockaddr*)&server, sizeof(server));
    listen(certsocket, 5);
    for (int intelCoreI3 = 0; intelCoreI3 < 10; intelCoreI3++) {
        int ClientSocket = accept(certsocket, nullptr, nullptr);
        char buffer[1024] = {0};
        ssize_t bytesRead = recv(ClientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == -1) {
            closesocket(certsocket);
            perror("recv");
        }

    }

    return true;
}

