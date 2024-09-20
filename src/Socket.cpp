//
// Created by ener9 on 18/09/2024.
//
#include <iostream>
#include <mutex>
#include "Socket.h"

#include <thread>


Socket& Socket::getInstance() {
    static Socket instance; // Guaranteed to be destroyed and instantiated on first use.
    return instance;
}

Socket::Socket() : serverSocket(INVALID_SOCKET), running(false) {
    std::cout << "Singleton Socket constructor created" << std::endl;
}

Socket::~Socket() {
    stop(); // Ensure server is stopped when the object is destroyed
}

void Socket::start(int port) {
    if (running) {
        std::cerr << "Server is already running." << std::endl;
        return;
    }

    // Initialize Winsock
    WSAData wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface
    serverAddr.sin_port = htons(port);

    // Bind the socket
    result = bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    // Start listening for incoming connections
    result = listen(serverSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "Socket Server started on port " << inet_ntoa(serverAddr.sin_addr) << ":"<< port << std::endl;
    std::thread ConnectionLoopThread(&Socket::SocketConnectionLoop, this);
    ConnectionLoopThread.detach();
    std::cin.get();
}
void Socket::SocketConnectionLoop() {
    while (true) {
        running = true;

        // Example: Accept clients (this could be in a loop, with thread handling, etc.)
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        // Handle the connected client
        std::thread handleClientThreadObject(handleClient, clientSocket);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        handleClientThreadObject.detach();
    }
}

void Socket::handleClient(int clientSocket) {
    std::string message = "Hello from the server!\n";
    send(clientSocket, message.c_str(), message.size(), 0);

    char buffer[512];
    int bytesReceived = recv(clientSocket, buffer, 512, 0);
    if (bytesReceived > 0) {
        std::cout << "Received message from client: " << std::string(buffer, 0, bytesReceived) << std::endl;
    }

    closesocket(clientSocket);

}

void Socket::stop() {
    if (!running) return;

    std::cout << "Stopping server..." << std::endl;
    closesocket(serverSocket);
    WSACleanup();
    running = false;
}
