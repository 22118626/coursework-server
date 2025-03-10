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
    this->dataStart =  static_cast<fpos_t>(readNextUint32_t());
    this->permissionLevel = readNextInt16_t();
    this->name = readNextString();
    //std::cout << "data->" << readNextString() <<"\t"<< readNextInt16_t() <<"<- end of data" << std::endl;
}


FileManager::~FileManager() {
    std::cout << "FileManager: " << this <<" Destructor called" << std::endl;
}

bool FileManager::openFile(const std::string& filePath) {
    std::cout << "(FM) opening file (" << filePath <<")"<<std::endl;
    try {
        if(std::filesystem::exists(filePath)) {
            FileStream.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
            return true;
        }
        //std::cout << "\"" << filePath << "\" does not exist" << std::endl;
        return false;
    } catch(const std::exception& exception) {
        return false;
    }
}

void FileManager::setPointerLoc(fpos_t addr) {
    this->pointerpos = addr;
    FileStream.seekg(addr);
    FileStream.seekp(addr);
}
uint8_t FileManager::readNextUint8_t() {
    uint8_t value;
    FileStream.read(reinterpret_cast<char*>(&value), sizeof(uint8_t));
    return value;
}
int8_t FileManager::readNextInt8_t() {
    int8_t value;
    FileStream.read(reinterpret_cast<char*>(&value), sizeof(int8_t));
    return value;
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
std::vector<uint8_t> FileManager::readBytes(unsigned int bytes) {
    //std::cout << "readBytes bytes: "<<bytes << std::endl <<"at pointer localtion: " << FileStream.tellg() << std::endl;
    std::vector<uint8_t> value(bytes);
    FileStream.read(reinterpret_cast<char *>(value.data()), bytes);
    return value;
}

// reads a null terminated string of arbitrary length. that has a max length.
std::string FileManager::readNextString() {
    uint16_t maxSizeOfString;
    fpos_t startStringPos = FileStream.tellg() + std::ifstream::pos_type(2);
    //std::cout << "startStringPos: " << startStringPos << std::endl;
    FileStream.read(reinterpret_cast<char*>(&maxSizeOfString), sizeof(uint16_t));
    std::vector<char> buffer(maxSizeOfString);

    FileStream.read(buffer.data(), maxSizeOfString);
    FileStream.seekg(startStringPos+ std::ifstream::pos_type(maxSizeOfString) , std::ios::beg);

    //std::cout << "maxSizeOfString: " << maxSizeOfString  << "\tnewpointerPos: " << FileStream.tellg() <<std::endl;
    return std::string(buffer.data());
    //return std::string without trailing 0x00
}

int FileManager::appendAtTheEnd(const std::vector<uint8_t> &data) {
    std::ofstream outfile(this->filePath, std::ios::binary | std::ios::app);
    if(!outfile) {
        std::cerr << "Error opening file for appending." << std::endl;
        return 1;
    }
    outfile.write(reinterpret_cast<const char*>(data.data()), data.size());
    if(!outfile.good()) {
        std::cout << "error when writing to file " << this->filePath << std::endl;
        return 2;
    }
    return 0;
}
int FileManager::modifyAtIndex(size_t index, const std::vector<uint8_t> &data) {
    std::ofstream outfile(this->filePath, std::ios::binary | std::ios::in | std::ios::out);
    if(!outfile) {
        std::cerr << "Error opening file for appending." << std::endl;
        return 1;
    }
    outfile.write(reinterpret_cast<const char*>(data.data()), data.size());
    if(!outfile.good()) {
        std::cout << "error when writing to file " << this->filePath << std::endl;
        return 2;
    }
    return 0;
}
int FileManager::modifyAtPointer(fpos_t pointer, const std::vector<uint8_t> &data) {
    setPointerLoc(pointer);
    FileStream.write(reinterpret_cast<const char*>(data.data()), data.size());
    std::cout << "pointer -- "<< pointer << "\ndata -- "; for (const auto& byte : data){
        std::cout<<std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    } std::cout<< std::endl;
    return 0;
}

int FileManager::ShiftDataFromfpos(fpos_t fpos, long shift) {
    //copy a temp file with same name and location but with ~ at the beginning of the name eg ".\gilbert.bdb" -> ".\~gilbert.bdb"
    std::string tempFile = (std::filesystem::path(this->filePath).parent_path() / ("~"+
                           std::filesystem::path(this->filePath).filename().string())).string();
    std::cout << "new file name is: " << tempFile << std::endl;
    closeFile();
    if (std::filesystem::exists(tempFile)) std::filesystem::remove(tempFile);
    std::filesystem::copy_file(this->filePath,tempFile,std::filesystem::copy_options::overwrite_existing);
    try {
        //  open the new file and modify it
        if (!openFile(tempFile)) {throw std::runtime_error("could not open file -> "+tempFile);};
        this->setPointerLoc(fpos);

        std::cout<<this->getFileSize()<<"\t"<<fpos<<" pointer:"<<this->getFileSize() - fpos<<std::endl;
        std::vector<uint8_t> movingBytes = readBytes(this->getFileSize() - fpos);
        std::filesystem::resize_file(std::filesystem::path(tempFile), file_size(std::filesystem::path(tempFile))+shift);
        if (shift > 0) { // oly if the data is shifted down/extended the file fill data from fpos to new index as 0s to prevent duplicate data
            std::vector<uint8_t> fillerData;
            fillerData.insert(fillerData.end(), shift, 0x00);
            modifyAtPointer(fpos, fillerData);
        }
        std::cout<<"record: " << std::endl;for (const auto& byte : movingBytes) {std::cout<< std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";}std::cout <<std::endl;

        // write the bytes
        modifyAtPointer(fpos + shift, movingBytes);

        if (!openFile(this->filePath)) {throw std::runtime_error("could not open file -> "+this->filePath);};
    }
    catch (const std::exception& ex) {
        std::cerr << "error occured trying to make changes to temporary file tempfile("<<tempFile<<")"<<std::endl<<ex.what()<<std::endl;
        return -1;
    }
    // copy successfully modified file back over the original one; and if copying was successful remove the tempFile;

    closeFile();
    std::cout << "ending" << std::endl;
    std::filesystem::remove(this->filePath);
    std::filesystem::rename(tempFile, this->filePath);
    openFile(this->filePath);
    std::cout << "tralala" << std::endl;
    return 0;
}

fpos_t FileManager::currentPointerPosition() {return FileStream.tellg();}

void FileManager::closeFile() {
    FileStream.close();
}
unsigned long long int FileManager::getFileSize() {
    return std::filesystem::file_size(this->filePath);
}

void FileManager::changeFilePath(const std::string &newPath) {
    this->filePath = newPath;
}


