//
// Created by ener9 on 29/09/2024.
//

#ifndef COURSEWORK_SERVER_DATABASEMANAGER_H
#define COURSEWORK_SERVER_DATABASEMANAGER_H
#include <string>
#include <fstream>


class DatabaseManager {

public:
    DatabaseManager();
    ~DatabaseManager();

    void openFile(std::string filePath);
    void readHeader();

private:
    fpos_t pointerpos{};
    std::ifstream dbFile;

    std::ifstream fileData;
    std::string readNextString();


};


#endif //COURSEWORK_SERVER_DATABASEMANAGER_H
