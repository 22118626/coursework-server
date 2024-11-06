//
// Created by ener9 on 29/09/2024.
//

#include "FileManager.h"

#include <cstdint>
#include <iostream>
#include <vector>
#include <filesystem>

FileManager::FileManager(const std::string& filePath) : filePath(filePath) {
    this->openFile(filePath);
}
FileManager::FileManager(const std::string& filePath,int i) : filePath(filePath){
}

void FileManager::readHeader() {
    dataStart =  static_cast<fpos_t>(readNextUint32_t());
    this->type = readNextInt16_t();
    this->name = readNextString();
    //std::cout << "data->" << readNextString() <<"\t"<< readNextInt16_t() <<"<- end of data" << std::endl;
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

void FileManager::setPointerLoc(fpos_t addr) {
    this->pointerpos = addr;
    FileStream.seekg(addr);
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
int32_t FileManager::readNextInt32_t() {
    int32_t value;
    FileStream.read(reinterpret_cast<char *>(&value), sizeof(int32_t));
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

void FileManager::appendAtTheEnd(const std::vector<uint8_t> &data) {
    std::ofstream outfile(this->filePath, std::ios::binary | std::ios::app);
    if(!outfile) {
        std::cerr << "Error opening file for appending." << std::endl;
        return;
    }
    outfile.write(reinterpret_cast<const char*>(data.data()), data.size());
    outfile.close();

    if(!outfile.good()) {
        std::cout << "error when writing to file " << this->filePath << std::endl;
    }
}

void FileManager::closeFile() {
    FileStream.close();
}

