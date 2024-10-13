//
// Created by 22118626 on 11/10/2024.
//

#include "CertSocket.h"
#include <iostream>
#include <fstream>
#include <vector>

CertSocket::CertSocket() : socketInstance(Socket::getInstance()){

}

CertSocket::~CertSocket() {
    if (certsocket != INVALID_SOCKET) {
        closesocket(certsocket);
    }
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
    }
}

enum class CertFiles : uint8_t {
    ServerCertificate,
    SecondCertificate,
    Count
};
const std::string availablefiles[] = {
        "ServerCertificate.crt",
        "secondCert.crt",
        "thirdCert.crt"
};

bool CertSocket::start(int port) {
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
        uint8_t buffer[16] = {0};
        ssize_t bytesRead = recv(ClientSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);

        if (bytesRead == -1) {
            closesocket(certsocket);
            perror("recv");
            continue;
        }

        auto fileRequest = static_cast<int>(buffer[0]) - 48;
        /*if (fileRequest < CertFiles::ServerCertificate || fileRequest > CertFiles::Count) {
            std::cerr << "Invalid file request:" << fileRequest << std::endl;
            closesocket(ClientSocket);
            continue;
        }*/
        std::cout << static_cast<int>(fileRequest) << " and " << availablefiles[fileRequest] << "\t\t\t" << availablefiles[0] << std::endl;
        sendCertificate(ClientSocket, availablefiles[fileRequest]);

    }

    return true;
}

void CertSocket::sendCertificate(SOCKET clientSocket, const std::string &certFile) {
    std::ifstream certfileStream(certFile, std::ios::binary);
    if(!certfileStream.is_open()) {
        std::cerr << "Failed to open certificate file: " << certFile << std::endl;
        return;
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(certfileStream)), std::istreambuf_iterator<char>());
    size_t filesize = buffer.size();
    std::cout <<"size: " << sizeof(filesize) << std::endl;
    send(clientSocket, reinterpret_cast<const char*>(filesize), sizeof(filesize), 0);

    //send actual data
    std::cout << buffer.data() << std::endl;
    send(clientSocket, buffer.data(), filesize, 0);

    std::string clientIP;
    uint16_t clientPort;
    std::cout << filesize << " sent certificate: " << certFile << " (" << filesize << " bytes) to client" << std::endl;
    closesocket(clientSocket);
}


CertSocket &CertSocket::getInstance() {
    static CertSocket instance;
    return instance;
}

