//
// Created by ener9 on 30/10/2024.
//

#ifndef COURSEWORK_SERVER_TABLE_H
#define COURSEWORK_SERVER_TABLE_H

#include <string>
#include <memory>
#include "Record.h"
#include "FileManager.h"


class Table {
public:
    std::string tableFilePath;

    explicit Table(const std::string& filePath);
    virtual ~Table()= default;
    virtual void addRecord(std::shared_ptr<Record> record);
    void printRecords() const;

   std::string tableName;

private:

    FileManager FM;

    void initializeTable();
    Record structureRecord;
    std::vector<int> recordFieldType;

    void addDataTypeToRecord(Record *record, const std::string &name, int type, int dataLength);
};


#endif //COURSEWORK_SERVER_TABLE_H
