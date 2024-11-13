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
    std::vector<FieldData> structureRecord; // arbitrary length array of [{String name, uint8 type, uint16 length}]
    Record record;
    int recordSize = 0;

    void addDataTypeToRecord(Record *record, const std::string &name, int type, int dataLength);

    Record readRecord(int index);
};


#endif //COURSEWORK_SERVER_TABLE_H
