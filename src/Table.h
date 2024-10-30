//
// Created by ener9 on 30/10/2024.
//

#ifndef COURSEWORK_SERVER_TABLE_H
#define COURSEWORK_SERVER_TABLE_H

#include <string>
#include <memory>
#include "Record.h"


class Table {

public:
    std::string tableFilePath;

    explicit Table(const std::string& filePath);
    virtual ~Table()= default;
    virtual void addRecord(std::shared_ptr<Record> record);
    void printRecords() const;


};


#endif //COURSEWORK_SERVER_TABLE_H
