//
// Created by ener9 on 30/10/2024.
//

#include "Table.h"
#include <vector>


enum recordTypes {
    Integer16B=1,
    Integer32B=2,
    String=3,
    reference=4
};


Table::Table(const std::string &filePath) :tableFilePath(filePath), FM(filePath,0) {
    std::cout << this->tableFilePath << std::endl;
    this->initializeTable();
}

void Table::initializeTable() {

    if (!this->FM.openFile(this->tableFilePath)) {
        std::cout << "oppening file (" << this->tableFilePath << ") failed" << std::endl;
        return;
    }

    this->FM.readHeader();
    fpos_t recordMetadataType = this->FM.readNextInt32_t();
    this->FM.setPointerLoc(recordMetadataType);
    int16_t NumOfFields = this->FM.readNextInt16_t();
    std::vector<std::string> fields(NumOfFields,"");
    this->tableName = this->FM.name;

    for(int i = 0; i < NumOfFields; i++) {
        fields[i] = this->FM.readNextString();
    }
    std::cout << this->tableName << std::endl << fields.data() << std::endl;
}

void Table::addRecord(std::shared_ptr<Record> record) {

}
