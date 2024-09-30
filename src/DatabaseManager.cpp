//
// Created by ener9 on 29/09/2024.
//

#include "DatabaseManager.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

DatabaseManager::DatabaseManager() {
    /*std::string string;
    dbFile.open("database.bin", std::ios::in | std::ios::binary);

    if (!dbFile.is_open()) {
        std::cerr << "Failed to open database file" << std::endl;
        return;
    }
    dbFile.seekg(0, std::ios::beg);
    uint16_t stringLength;
    dbFile.read(reinterpret_cast<char*>(&stringLength), sizeof(uint16_t));
    std::vector<char> buffer(stringLength);
    std::cout << "idk" << stringLength << std::endl;
    dbFile.read((buffer.data()), stringLength);
    dbFile.tellg();
    dbFile.close();
    std::cout << "string in the file: " << buffer.data() << std::endl;*/
    openFile("database.bin");
    std::cout << readNextString() << std::endl;
    dbFile.close();

}

DatabaseManager::~DatabaseManager() {
    std::cout << "Destructor called" << std::endl;
}

void DatabaseManager::openFile(std::string fileName) {
    dbFile.open(fileName, std::ios::in | std::ios::binary);
}

std::string DatabaseManager::readNextString() {
    int16_t stringLength;
    dbFile.read(reinterpret_cast<char*>(&stringLength), sizeof(uint16_t));
    std::vector<char> buffer(stringLength);
    std::cout << stringLength << std::endl;
    dbFile.read(buffer.data(), sizeof(stringLength));
    return buffer.data();
}




