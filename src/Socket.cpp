//
// Created by ener9 on 18/09/2024.
//


#include "Socket.h"
#include <chrono>

std::shared_ptr<Database> Socket::db = nullptr;
Socket& Socket::getInstance() {
    static Socket instance;
    return instance;
}

Socket::Socket() : serverSocket(INVALID_SOCKET), running(false), ctx(nullptr) {
    std::cout << "Singleton Socket constructor created" << std::endl;

    // Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    db = Database::GetInstance();
    std::cout << "out of DB in Socket" << std::endl;
}

Socket::~Socket() {
    stop();
    SSL_CTX_free(ctx);
    EVP_cleanup();
}

bool Socket::start(int port) {
    if (running) {
        std::cerr << "Server is already running." << std::endl;
        return false;
    }

    // Initialize Winsock
    WSAData wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }

    // Create SSL context
    ctx = SSL_CTX_new(TLS_server_method()); // Create SSL context
    if (!ctx) {
        std::cerr << "Unable to create SSL context" << std::endl;
        return false;
    }

    // Load server certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, "ServerCertificate.crt", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Unable to load certificate or key please create one in the window that will soon appear then run the command aggain" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds (5));
        //executes a cmd command to create a key and then asks user to fillout form to create a certificate, to be used for ssl/tls encrypted packets between client and server.
        int cmdresult = system("cmd.exe /c echo the files will be created in \"%cd%\" && "
                               ".\\OpenSSL-Win64\\bin\\openssl.exe genrsa -out server.key 2048 &&"
                               "set OPENSSL_CONF=%cd%\\OpenSSL-Win64\\bin\\cnf\\openssl.cnf &&"
                               ".\\OpenSSL-Win64\\bin\\openssl.exe req -x509 -key server.key -out ServerCertificate.crt");
        if(cmdresult > 0) {
            std::cout << "':('  😢" << std::endl;
        }
        return false;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
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
        return false;
    }

    // Start listening for incoming connections
    result = listen(serverSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    std::cout << "Socket Server started on port " << port << std::endl;

    // Start connection loop in a separate thread
    running = true;
    std::thread connectionLoopThread(&Socket::SocketConnectionLoop, this);
    connectionLoopThread.detach(); // Detach the thread to allow it to run independently

    // Wait for user input to keep the server running
    //std::cin.get(); // <- redundant. causes a bug where if the socket is created, and command is typed by the used this line will hijack the first character ("exit" -> "xit" => no command 'xit')
    return true;
}

void Socket::SocketConnectionLoop() {
    while (running) {
        // Accept clients
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue; // Continue accepting other clients
        }

        // Create SSL object and attach it to the client socket
        SSL* ssl = SSL_new(ctx);
        SSL_set_fd(ssl, clientSocket);

        // Perform SSL handshake
        if (SSL_accept(ssl) <= 0) {
            std::cerr << "SSL_accept failed: " << ERR_get_error() << std::endl;
            SSL_free(ssl);
            closesocket(clientSocket);
            continue;
        }

        // Handle the connected client in a separate thread
        std::thread handleClientThread([this, ssl, clientSocket]() {handleClient(ssl, clientSocket);});
        handleClientThread.detach(); // Detach the thread to allow it to run independently
    }
}

void Socket::handleClient(SSL* ssl, SOCKET clientSocket) {

    // Receive a message from the client
    char buffer[2048];
    int bytesReceived = SSL_read(ssl, buffer, sizeof(buffer)); // Use SSL_read instead of recv
    if (bytesReceived > 0) {
        std::cout << "Received message from client: "
                  << std::string(buffer, bytesReceived) << std::endl;
    }
    // Send a welcome message to the client
    const std::string message = db->parseDatabaseCommand(std::string(buffer)).dump();
    SSL_write(ssl, message.c_str(), message.size());

    // Shutdown the SSL connection and close the socket
    SSL_shutdown(ssl);
    SSL_free(ssl);
    closesocket(clientSocket);
}

void Socket::stop() {
    if (!running) return;

    std::cout << "Stopping server..." << std::endl;
    closesocket(serverSocket);
    WSACleanup();
    running = false;
    std::cout << "Server stopped." << std::endl;
}
