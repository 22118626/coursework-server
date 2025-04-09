//
// Created by ener9 on 18/09/2024.
//

#include "CLIApp.h"
#include <sstream>
#include <iostream>
#include <cmath>

#include "Database.h"
#include "Table.h"

namespace fs = std::filesystem;

// uses formula x + baseValue + (x + multiplier) to make a max length of a string to allow for small changes after the table is created.
uint16_t maxStringLength(int input) {
    constexpr int baseValue = 7;
    constexpr float multiplier = 0.1;
    const uint16_t value = input + baseValue + static_cast<int>(std::ceil(input * multiplier));
    return value;
}

std::string padString(std::string string, int targetLength) {
    if (targetLength - string.length() > 0) {
        string.append(targetLength - string.length(), '\0');
    }
    return string;
}
// initializes basic commands by linking methods to individual keywords and pass the arguments to those functions if needed
CLIApp::CLIApp() {
    commands["connect"] = [this](const std::string& args) {this->ConnectToSocket(args);};
    commands["stop"] = [this](const std::string&) {this->stop();};
    commands["help"] = [this](const std::string&) {this->help();};
    commands["exit"] = [this](const std::string&) {this->exit();};
    commands["checkdb"] = [this](const std::string&) {this->checkdb();};
    commands["certServer"] = [this](const std::string& args) {this->certsocket(args);};
    commands["TableTest"] = [this](const std::string& args) {this->TableTest(args);};
    commands["emulateIncomingConnection"] = [this](const std::string& args) {this->emulateDbConnection(args);};
    commands["createTable"] = [this](const std::string&) {this->createTable();};
    commands["backup"] = [this](const std::string& args) {CLIApp::Backup(args);};
    // enable a looping condition to allow for constantly listening for new commands sent by the user
    this->running=true;
}

void CLIApp::run() {
    // initial boot-up message :)
    std::cout << "Hello World!" << std::endl;
    while(this->running) {
        // without another library imported into the project(and the school refusing to let me install it on the computer)
        // this is the best I can do ↴  having a ">>" on the latest line would indicate a User Input Cue
        std::string input;
        // prints ">>" and waits with std::getline until a user enters information and presses enter
        std::cout << ">>"; std::getline(std::cin, input);

        // splits the command into the root/operator and arguments to fine tune the execution
        std::istringstream iss(input);
        std::string command;
        std::string args;

        iss >> command;
        std::getline(iss, args);
        // itterates through the this->commands array until the right one is found and executes it
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
    // guard clause
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
    // validity check that a port is within 0 and (2**16)-1
    if(port == 0 || port > std::pow(2,16) - 1) {
        std::cout << "invalid args, use: connect <port>" << std::endl;
        return;
    }

    // get the instance of the cert socket (if it does not exist it will be automaically created) and start it at the user given port number
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
    FileManager fileManager("file.bin");
    fileManager.readHeader();
    fileManager.closeFile();

}

void CLIApp::help() {
    /*std::cout << std::left << std::setw(32) << "command" << std::setw(64) <<"description" << std::endl <<
    std::setw(32) << "connect <port>" << std::setw(64) <<"Used to start the listening socket with <port>" << std::endl <<
    std::setw(32) << "stop" << std::setw(64) <<"stops the socket from running" << std::endl <<
    std::setw(32) << "exit" << std::setw(64) <<"exits the program" << std::endl;*/

    // print all commands to console
    for (const auto &pair : commands) {
        const std::string &commandName = pair.first;
        std::cout << commandName << std::endl;
    }
}



// debug commands
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
    auto returns = db->parseDatabaseCommand(nlohmann::json::parse(map["arg"]), 0);
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
        if (argument.starts_with("--")) {
            currentFlag = argument.substr(2);
        }
        else if (argument.starts_with("-") && argument.size() > 1) {
            currentFlag = argument.substr(1);
        }
        else if (!currentFlag.empty()) {
            commandpairs[currentFlag] = argument;
            currentFlag.clear();
        }
    }
    if (!currentFlag.empty()) {
        commandpairs[currentFlag] = "";
    }
    return commandpairs;
}

void CLIApp::createTable() {
    std::string input;

    std::cout << "table name:"; std::getline(std::cin, input);
    std::string tableName = input;
    if (input.empty()) {std::cout <<"input empty quiting"<<std::endl;return;}
    //create the file
    std::fstream file("./tables/" + input + ".bdb", std::ios::app);
    file.close();
    // open the file with in/out binary
    std::fstream output("./tables/" + input + ".bdb", std::ios::in | std::ios::out | std::ios::binary);

    if (!output) {std::cerr << "failed to open or create file" << std::endl;return;}
    // beginning of the file
    output.seekp(0, std::ios::beg);
    // temporarely set beginning of "dataStart to pointer 1024
    uint32_t datastart = 1024;
    output.write(reinterpret_cast<char*>(&datastart), sizeof(uint32_t));
    std::cout << std::endl << "permission level (0 = everyone) : "; std::getline(std::cin, input);
    if (input.empty()) {std::cout <<"input empty quiting"<<std::endl;return;}
    try {
        // write permissionlevel to file
        int tmp = std::stoi(input);
        output.write(reinterpret_cast<char*>(&tmp), sizeof(uint16_t));
    } catch (std::exception &e) {
        std::cout <<"\"" <<input << "\" is not a valid number" << std::endl;
    }
    // write name of the table to file
    uint16_t maxstringlength = maxStringLength(tableName.length());
    output.write(reinterpret_cast<char*>(&maxstringlength), sizeof(uint16_t));
    output.write(padString(tableName, maxstringlength).data(), maxstringlength);
    // 0 because no records will be in the table
    int tmp = 0;
    output.write(reinterpret_cast<char*>(&tmp), sizeof(uint32_t));
    std::cout << std::endl << "how many fields are in the table? (first is primary and must be uint32_t):"; std::getline(std::cin, input);
    if (input.empty()) {std::cout <<"input empty quiting"<<std::endl;return;}
    try {
        int tmp = std::stoi(input);
        output.write(reinterpret_cast<char*>(&tmp), sizeof(uint16_t));
        std::string inp;
        for (int i = 0 ; i < std::stoi(input) ; i++) {
            std::cout << std::endl << "what is the name of the field? : "; std::getline(std::cin, inp);
            if (inp.empty()) {std::cout <<"input empty quiting"<<std::endl;return;}
            uint16_t tmp = maxStringLength(inp.length());
            output.write(reinterpret_cast<char*>(&tmp), sizeof(uint16_t));
            output.write(padString(inp, maxStringLength(inp.length())).data(), maxStringLength(inp.length()));
            std::cout << std::endl << "what is the type of the field?\n1 = uint16_t\n2 = uint32_t\n3 = string\n4 = foreign key";
            std::getline(std::cin, inp);
            if (inp.empty()) {std::cout <<"input empty quiting"<<std::endl;return;}
            try {
                int tmp = std::stoi(inp);
                output.write(reinterpret_cast<char*>(&tmp), sizeof(uint8_t));

            } catch (std::exception &e) {
                std::cerr <<"\"" <<input << "\" is not a valid number. \nexiting" << std::endl;
            }
            if (std::stoi(inp) == 1) {
                int tmp = 2;
                output.write(reinterpret_cast<char*>(&tmp), sizeof(uint16_t));
            }if (std::stoi(inp) == 2) {
                int tmp =4;
                output.write(reinterpret_cast<char*>(&tmp), sizeof(uint16_t));
            }if (std::stoi(inp) == 3) {
                std::string fieldlengthinput;
                std::cout << std::endl << "what is the max length of the string? : "; std::getline(std::cin, fieldlengthinput);
                int tmp = std::stoi(fieldlengthinput);
                try {output.write(reinterpret_cast<char*>(&tmp), sizeof(uint16_t));}
                catch (std::exception &e) {std::cerr <<"\"" <<input << "\" is not a valid number. \nexiting" << std::endl;return;}
            }if (std::stoi(inp) == 4) {
                int tmp = 4;
                output.write(reinterpret_cast<char*>(&tmp), sizeof(uint16_t));
                std::string tablenameinput;
                std::cout << std::endl << "what table name is this linking to? : "; std::getline(std::cin, tablenameinput);
                tmp = maxStringLength(tablenameinput.length());
                output.write(reinterpret_cast<char*>(&tmp), sizeof(uint16_t));
                output.write(padString(tablenameinput, maxStringLength(tablenameinput.length())).data(), maxStringLength(tablenameinput.length()));
            }
        }
    } catch (std::exception &e) {
        std::cout <<"\"" <<input << "\" is not a valid number" << std::endl;
    }
    int lastbyte = output.tellp();
    int newDataStart = maxStringLength(lastbyte);
    std::string zeroing = "";
    zeroing = padString(zeroing, newDataStart-lastbyte);
    output.write(zeroing.data(), zeroing.length());
    output.seekp(0, std::ios::beg);
    output.write(reinterpret_cast<char*>(&newDataStart), sizeof(uint32_t));

    std::cout << std::endl << "\n\nfinished writing all fields\nsuccessfully created the file :)" << std::endl;
    output.close();
}


void CLIApp::Backup(const std::string& args) {
    std::unordered_map<std::string, std::string> cmds = CLIApp::CommandParser(args);
    for (auto pair : cmds) {
        if ( pair.first == "b" || pair.first == "backup") {
            auto now = std::chrono::system_clock::now();
            std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
            std::tm now_tm = *std::localtime(&now_time_t);
            std::ostringstream oss;
            oss << std::put_time(&now_tm, "Backup-%d-%m-%Y-%H-%M");
            std::string newDir = pair.second.empty() ?  oss.str() : pair.second;
            fs::create_directory(".\\tables\\"+newDir);
            for (std::filesystem::directory_entry file : fs::directory_iterator(".\\tables\\")) {
                if (file.is_regular_file() && file.path().extension().string() == ".bdb" && !(file.path().filename().string().starts_with(".") || file.path().filename().string().starts_with("~"))) {
                    fs::copy(file, ".\\tables\\"+newDir+"\\"+file.path().filename().string(), fs::copy_options::overwrite_existing);
                }
            }
            std::cout<<"backup success!"<<std::endl;
        }else if ( pair.first == "r" || pair.first == "restore") {
            std::string newDir = pair.second;
            for (std::filesystem::directory_entry file : fs::directory_iterator(".\\tables\\"+newDir)) {
                if (file.is_regular_file() && file.path().extension().string() == ".bdb" && !(file.path().filename().string().starts_with(".") || file.path().filename().string().starts_with("~"))) {
                    fs::remove(".\\tables\\"+file.path().filename().string());
                    fs::copy(file, ".\\tables\\"+file.path().filename().string(), fs::copy_options::overwrite_existing);
                }
            }
            std::cout<<"restore success!"<<std::endl;
        }
        else if (pair.first == "l" || pair.first == "list") {
            std::cout << "List of backups:" << std::endl;
            for (const auto& directory : fs::directory_iterator(".\\tables\\")) {
                if (directory.is_directory()) {
                    int count = 0; for(const auto& entry:fs::directory_iterator(directory)) {entry.is_regular_file() ? count++ : count;}
                    std::cout << directory.path().filename().string() << "\t\tfile count: " << count << std::endl;
                }
            }
        }else if (pair.first == "d" || pair.first == "delete"){
            for (const auto& directory : fs::directory_iterator(".\\tables\\")) {
                if (directory.is_directory() && directory.path().filename().string() == pair.second) {
                    FileManager::recursiveRemove(".\\tables\\"+directory.path().filename().string());
                }
            }
        }else {
            std::cout << pair.first << " is not a valid command\n"  << "Use a valid format: backup <-b | -backup | -r | -restore>"<< std::endl;
        }
    }
}
