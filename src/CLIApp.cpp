//
// Created by ener9 on 18/09/2024.
//

#include "CLIApp.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>

#include "Database.h"
#include "Table.h"



CLIApp::CLIApp() {
    commands["connect"] = [this](const std::string& args) {this->ConnectToSocket(args);};
    commands["stop"] = [this](const std::string&) {this->stop();};
    commands["help"] = [this](const std::string&) {this->help();};
    commands["exit"] = [this](const std::string&) {this->exit();};
    commands["checkdb"] = [this](const std::string&) {this->checkdb();};
    commands["certServer"] = [this](const std::string& args) {this->certsocket(args);};
    commands["TableTest"] = [this](const std::string& args) {this->TableTest(args);};
    commands["emulateIncomingConnection"] = [this](const std::string& args) {this->emulateDbConnection(args);};
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
    if(!this->socket->start(port)) {
        std::cout << "failed to start socket make sure SSL certificate and key are correctly set up" << std::endl;
        return false;
    }
    return true;
}

void CLIApp::certsocket(const std::string& args) {
    std::istringstream iss(args);
    int port = 0;

    iss >> port;
    if(port == 0 || port > std::pow(2,16) - 1) {
        std::cout << "invalid args, use: connect <port>" << std::endl;
        return;
    }


    CertSocket& certSocket = CertSocket::getInstance();
    if(!certSocket.start(port)) {
        std::cout << "failed to start socket make sure SSL certificate and key are correctly set up" << std::endl;
    }
}

void CLIApp::exit() {
    this->stop();
    this->running = false;
}
void CLIApp::checkdb() {
    std::cout << "checkdb" << std::endl;
    FileManager fileManager("file.bin");
    fileManager.readHeader();
    fileManager.closeFile();

}

void CLIApp::help() {
    /*std::cout << std::left << std::setw(32) << "command" << std::setw(64) <<"description" << std::endl <<
    std::setw(32) << "connect <port>" << std::setw(64) <<"Used to start the listening socket with <port>" << std::endl <<
    std::setw(32) << "stop" << std::setw(64) <<"stops the socket from running" << std::endl <<
    std::setw(32) << "exit" << std::setw(64) <<"exits the program" << std::endl;*/

    for (const auto &pair : commands) {
        const std::string &commandName = pair.first;
        std::cout << commandName << std::endl;
    }
}

void CLIApp::TableTest(const std::string& args) {
    auto map = CommandParser(args);
    std::string file;
    Table table;

    for (auto pair : map) {
        std::cout << pair.first << "  " << pair.second << std::endl;
        if (pair.first == "f" || pair.first == "file") {
            file = pair.second;
            if (!file.empty()) {table.setFilePath(file); table.initializeTable();}
        }else if(pair.first =="search") {
            std::string val1,val2; std::stringstream ss(pair.second); std::getline(ss,val1,',');std::getline(ss,val2);
            table.debugSearch(val1, val2);
        }
    }

}

void CLIApp::emulateDbConnection(const std::string& args) {
    std::shared_ptr<Database> db = Database::GetInstance();
    auto map = CommandParser(args);
    std::cout << map["arg"] << std::endl;
    auto returns = db->parseDatabaseCommand(map["arg"]);
    std::cout << returns.dump(2) << std::endl;
}



/* returns a key-value pair table to the caller after parsing individual flags and their following value pair, the function
 * returns an empty string ("") if no value is associated with the flag*/
std::unordered_map<std::string, std::string> CLIApp::CommandParser(const std::string &args) {
    std::istringstream iss(args);
    std::string argument;
    std::string currentFlag;
    std::unordered_map<std::string, std::string> commandpairs;

    while (iss >> argument) {
        if (argument.starts_with("--"))     currentFlag = argument.substr(2);
        else if (argument.starts_with("-") && argument.size() > 1)       currentFlag = argument.substr(1);
        else if (!currentFlag.empty())      commandpairs[currentFlag] = argument;
        else    commandpairs[currentFlag] = "";

    }
    return commandpairs;
}
