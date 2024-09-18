//
// Created by ener9 on 18/09/2024.
//

#include "CLIApp.h"

CLIApp::CLIApp() {
    commands["connect"] = [this]() {this->ConnectToSocket();};
    commands["stop"] = [this]() {this->stop();};
}

void CLIApp::run() {
    this->ConnectToSocket();

}
void CLIApp::stop() {
    socket->stop();
}

bool CLIApp::ConnectToSocket() {
    this->socket = &Socket::getInstance();
    this->socket->start(25565);
    return false;
}

