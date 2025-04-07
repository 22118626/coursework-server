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
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return false;
    }

    std::cout << "CertSocket started at port " << port << std::endl;
    certsocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(certsocket, reinterpret_cast<sockaddr *>(&server), sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (listen(certsocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    running = true;
    std::thread connectionLoopThread(&CertSocket::ConnectionLoop, this);
    connectionLoopThread.detach();

    return true;
}

void CertSocket::ConnectionLoop() {
    while (running) {
        //std::cout << "running" << std::endl;
        int ClientSocket = accept(certsocket, nullptr, nullptr);
        //std::cout << "accept: " << ClientSocket << std::endl;
        uint8_t buffer[16] = {0};
        ssize_t bytesRead = recv(ClientSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);

        if (bytesRead == -1) {
            closesocket(certsocket);
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
    send(clientSocket, buffer.data(), filesize, 0);

    std::string clientIP;
    uint16_t clientPort;
    std::cout << filesize << " sent certificate: " << certFile << " (" << filesize << " bytes) to client" << std::endl;
    closesocket(clientSocket);
}

void CertSocket::stop() {
    running = false;
}


CertSocket &CertSocket::getInstance() {
    static CertSocket instance;
    return instance;
}

