//
// Created by ener9 on 29/09/2024.
//

#ifndef COURSEWORK_SERVER_FILEMANAGER_H
#define COURSEWORK_SERVER_FILEMANAGER_H
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>


class FileManager {

public:
    FileManager();
    FileManager(const std::string &filePath);
    FileManager(const std::string &filePath, int i);

    ~FileManager();

    bool openFile(const std::string& filePath);
    void readHeader();
    int8_t readNextInt8_t();
    uint8_t readNextUint8_t();
    int16_t readNextInt16_t();
    uint16_t readNextUint16_t();
    int32_t readNextInt32_t();
    uint32_t readNextUint32_t();
    std::string readNextString();
    bool readNextBool();
    void closeFile();

    void appendAtTheEnd(const std::vector<uint8_t>& data);

    fpos_t dataStart{};
    std::uint16_t type;
    std::string name;
    void setPointerLoc(fpos_t addr);

    fpos_t currentPointerPosition();

private:
    fpos_t pointerpos{};
    std::ifstream FileStream;

    std::ifstream fileData;
    const std::string &filePath;

};


#endif //COURSEWORK_SERVER_FILEMANAGER_H
