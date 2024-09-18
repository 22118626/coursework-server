//
// Created by ener9 on 18/09/2024.
//

#ifndef COURSEWORK_SERVER_CLIAPP_H
#define COURSEWORK_SERVER_CLIAPP_H

#include <string>
#include <unordered_map>
#include <functional>
#include "Socket.h"


class CLIApp {
public:
    CLIApp();
    void run();

private:
    bool running;

    void stop();
    bool ConnectToSocket();
    Socket* socket;

    std::unordered_map<std::string, std::function<void()>> commands;
};


#endif //COURSEWORK_SERVER_CLIAPP_H
