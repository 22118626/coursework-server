//
// Created by ener9 on 29/09/2024.
//

#ifndef COURSEWORK_SERVER_FILEMANAGER_H
#define COURSEWORK_SERVER_FILEMANAGER_H
#include <cstdint>
#include <string>
#include <fstream>


class FileManager {

public:
    FileManager(std::string path);
    ~FileManager();

    void openFile(const std::string& filePath);
    void readHeader();
    int16_t readNextInt16_t();
    uint16_t readNextUint16_t();
    int32_t readNextInt32_t();
    uint32_t readNextUint32_t();
    std::string readNextString();
    bool readNextBool();

private:
    fpos_t dataStart;
    fpos_t pointerpos{};
    std::ifstream FileStream;

    std::ifstream fileData;


};


#endif //COURSEWORK_SERVER_FILEMANAGER_H
