//
// Created by ener9 on 29/09/2024.
//

#include "FileManager.h"

#include <cstdint>
#include <iostream>
#include <vector>

FileManager::FileManager(std::string path) {
    openFile(path);
    std::cout << "data->" << readNextString() <<"\t"<< readNextInt16_t() <<"<- end of data" << std::endl;
    FileStream.close();

}

void FileManager::readHeader() {
    dataStart =  static_cast<fpos_t>(readNextUint32_t());
}


FileManager::~FileManager() {
    std::cout << "Destructor called" << std::endl;
}

void FileManager::openFile(const std::string& filePath) {
    FileStream.open(filePath, std::ios::in | std::ios::binary);
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
    return value;
}


std::string FileManager::readNextString() {
    uint16_t stringLength;
    FileStream.read(reinterpret_cast<char*>(&stringLength), sizeof(uint16_t));
    std::vector<char> buffer(stringLength);
    FileStream.read(buffer.data(), stringLength);
    FileStream.seekg(FileStream.tellg()+ std::ifstream::pos_type(1) , std::ios::beg);
    return buffer.data();
}




