//
// Created by ener9 on 18/09/2024.
//

#include "CLIApp.h"
#include <sstream>
#include <iostream>
#include <iomanip>

#include "FileManager.h"

CLIApp::CLIApp() {
    commands["connect"] = [this](const std::string& args) {this->ConnectToSocket(args);};
    commands["stop"] = [this](const std::string&) {this->stop();};
    commands["help"] = [this](const std::string&) {this->help();};
    commands["exit"] = [this](const std::string&) {this->exit();};
    commands["checkdb"] = [this](const std::string&) {this->checkdb();};
    this->running=true;
}

void CLIApp::run() {
    std::cout << "Hello World!" << std::endl;
    while(this->running) {
        std::string input;
        std::cout << ">>"; std::getline(std::cin, input);

        std::istringstream iss(input);
        std::string command;
        std::string args;

        iss >> command;
        std::getline(iss, args);

        auto it = commands.find(command);
        if(it != commands.end()) {
            it->second(args);
        }
        else {
            std::cout << "unknown command '" << command << "'" << std::endl;
        }
    }
}
void CLIApp::stop() {
    socket->stop();
}

bool CLIApp::ConnectToSocket(const std::string& args) {
    std::istringstream iss(args);
    int port = 0;

    iss >> port;
    if(port == 0) {
        std::cout << "invalid args, use: connect <port>" << std::endl;
        return false;
    }


    std::cout << "connect " << port << std::endl;

    this->socket = &Socket::getInstance();
    this->socket->start(port);
    return true;
}

void CLIApp::exit() {
    this->stop();
    this->running = false;
}
void CLIApp::checkdb() {
    std::cout << "checkdb" << std::endl;
    FileManager file = FileManager(std::string("file.bin"));
    file.readHeader();
    file.closeFile();
}

void CLIApp::help() {
    std::cout << std::left << std::setw(32) << "command" << std::setw(64) <<"description" << std::endl <<
    std::setw(32) << "connect <port>" << std::setw(64) <<"Used to start the listening socket with <port>" << std::endl <<
    std::setw(32) << "stop" << std::setw(64) <<"stops the socket from running" << std::endl <<
    std::setw(32) << "exit" << std::setw(64) <<"exits the program" << std::endl;
}

