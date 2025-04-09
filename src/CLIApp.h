//
// Created by ener9 on 18/09/2024.
//

#ifndef COURSEWORK_SERVER_CLIAPP_H
#define COURSEWORK_SERVER_CLIAPP_H

#include <string>
#include <unordered_map>
#include <functional>

#include "Socket.h"
#include "CertSocket.h"
#include "FileManager.h"


class CLIApp {
public:
    CLIApp();
    void run();

private:
    bool running;

    void stop();
    bool ConnectToSocket(const std::string& arg);
    Socket* socket;

    std::unordered_map<std::string, std::function<void(const std::string&)>> commands;

    void help();
    void exit();
    void checkdb();
    void certsocket(const std::string& arg);
    void TableTest(const std::string& args);
    static std::unordered_map<std::string, std::string> CommandParser(const std::string &args);
    void createTable();

    static void Backup(const std::string &args);

    static void emulateDbConnection(const std::string& args);

};


#endif //COURSEWORK_SERVER_CLIAPP_H
