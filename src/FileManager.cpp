//
// Created by ener9 on 29/09/2024.
//

#include "FileManager.h"

#include <cstdint>
#include <iostream>
#include <vector>
#include <filesystem>

FileManager::FileManager() {

}

void FileManager::readHeader() {
    dataStart =  static_cast<fpos_t>(readNextUint32_t());
    std::cout << "data->" << readNextString() <<"\t"<< readNextInt16_t() <<"<- end of data" << std::endl;
}


FileManager::~FileManager() {
    std::cout << "Destructor called" << std::endl;
}

bool FileManager::openFile(const std::string& filePath) {
    try {
        if(std::filesystem::exists(filePath)) {
            FileStream.open(filePath, std::ios::in | std::ios::binary);
            return true;
        }
        std::cout << "\"" << filePath << "\" does not exist" << std::endl;
        return false;
    } catch(const std::exception& exception) {
        return false;
    }
}

int16_t FileManager::readNextInt16_t() {
    int16_t value;
    FileStream.read(reinterpret_cast<char*>(&value), sizeof(int16_t));
    return value;
}
uint16_t FileManager::readNextUint16_t() {
    uint16_t value;
    FileStream.read(reinterpret_cast<char*>(&value), sizeof(uint16_t));
    return value;
}
uint32_t FileManager::readNextUint32_t() {
    uint32_t value;
    FileStream.read(reinterpret_cast<char*>(&value), sizeof(uint32_t));
    std::cout << FileStream.tellg() << std::endl;
    return value;
}


std::string FileManager::readNextString() {
    uint16_t stringLength;
    std::cout << FileStream.tellg() << std::endl;
    FileStream.read(reinterpret_cast<char*>(&stringLength), sizeof(uint16_t));
    std::vector<char> buffer(stringLength);
    FileStream.read(buffer.data(), stringLength);
    FileStream.seekg(FileStream.tellg()+ std::ifstream::pos_type(1) , std::ios::beg);
    return buffer.data();
}

void FileManager::closeFile() {
    FileStream.close();
}

