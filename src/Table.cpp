//
// Created by ener9 on 30/10/2024.
//

#include "Table.h"
#include "FileManager.h"


Table::Table(const std::string &filePath) {
    this->tableFilePath = filePath;

    FileManager FM(this->tableFilePath);
    FM.readHeader();

}

void Table::addRecord(std::shared_ptr<Record> record) {

}
