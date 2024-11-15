//
// Created by ener9 on 30/10/2024.
//

#include "Table.h"
#include <vector>
#include <strings.h>
#include <iomanip>


/*enum recordTypes {
    Integer16B=1,
    Integer32B=2,
    String=3,
    reference=4
    bool=5
};
 */



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
    //fpos_t recordMetadataType = this->FM.readNextInt32_t();
    //this->FM.setPointerLoc(recordMetadataType);
    int16_t NumOfFields = this->FM.readNextInt16_t();
    std::vector<std::string> fields(NumOfFields,"");
    this->tableName = this->FM.name;
    std::cout << this->FM.currentPointerPosition() << std::endl;
    //std::cout << this->tableName << std::endl << fields.data() << std::endl;
    std::cout << "DataStart: " << this->FM.dataStart << std::endl <<
    "Type: " << this->FM.type << std::endl <<
    "name: " << this->FM.name << std::endl <<
    "numberOfFields: " << NumOfFields << std::endl;

    /*LoginID uint32| Username String(50) | HashedPassword String(64Bytes/256bits)*/
    for(int i = 0; i < NumOfFields; i++) {
        FieldData fieldData = {this->FM.readNextString(), this->FM.readNextUint8_t(),this->FM.readNextUint16_t()};
        structureRecord.push_back(fieldData);
        std::cout << this->FM.currentPointerPosition() << std::endl;
    }
    this->recordSize = 0;
    for ( auto i : structureRecord) {
        std::cout << "name: " << i.name << "\tlength: " << i.length << std::endl;
        this->recordSize += i.length;
    }
    std::cout << "Finished reading record metadata. RecordSize: " << this->recordSize << std::endl;
    /*Record::printRecords(this->structureRecord,this->recordFieldType) << std::endl;
    record1.record.push_back(std::make_shared<Int32Field>("LoginID", 0x2052));
    std::cout << typeid(*record1.record.at(0)).name() << std::endl;
    record1.record.push_back(std::make_shared<StringField>("Username", "0x2052", 50));
    record1.record.push_back(std::make_shared<StringField>("HashedPassword", "0x2052fc64cb3a", 128));
    std::cout << std::to_string(record1.returnvalueFromindex<Int32Field>(structureRecord.fieldName("LoginID")).value);*/
    Record recordIndexed1 = this->readRecord(0);

}


void Table::addRecord(std::shared_ptr<Record> record) {

}

Record Table::readRecord(int index) {
    this->FM.setPointerLoc(this->FM.dataStart + index * this->recordSize);
    auto vec = this->FM.readBytes(recordSize);
    for (auto byte : vec) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::endl;
    int offset = 0;int offset = 0;
    for(auto field : this->structureRecord) {
        /*std::cout << field.name << std::endl;
        std::cout << field.length << std::endl;
        std::cout << field.type << std::endl;*/
        std::cout << "offset: " << offset << std::endl;

        if(field.type == 4 || field.type == 3) {
            std::cout << field.name << " :" << std::string(reinterpret_cast<const char*>(vec.data()) + offset, field.length) << std::endl;
        } else if (field.type == 2 || field.type == 1) { // Numeric type
            if (field.length == 4) { //int32_t
                int value = *reinterpret_cast<const int*>(vec.data() + offset);
                std::cout << "value: " << std::dec << value << std::endl;
            } else if(field.length == 2) {//int16_t
                int value = *reinterpret_cast<const short*>(vec.data() + offset);
                std::cout << "value: " << std::dec << value << std::endl;
            }
            else {
                std::cout << "[Unsupported numeric length]";
            }
        } else {
            std::cout << "[Unknown field type]";
        }
        offset += field.length;
    }
    for(auto field : this->structureRecord) {
        /*std::cout << field.name << std::endl;
        std::cout << field.length << std::endl;
        std::cout << field.type << std::endl;*/
        std::cout << "offset: " << offset << std::endl;

        if(field.type == 4 || field.type == 3) {
            std::cout << field.name << " :" << std::string(reinterpret_cast<const char*>(vec.data()) + offset, field.length) << std::endl;
        } else if (field.type == 2 || field.type == 1) { // Numeric type
            if (field.length == 4) { //int32_t
                int value = *reinterpret_cast<const int*>(vec.data() + offset);
                std::cout << "value: " << std::dec << value << std::endl;
            } else if(field.length == 2) {//int16_t
                int value = *reinterpret_cast<const short*>(vec.data() + offset);
                std::cout << "value: " << std::dec << value << std::endl;
            }
            else {
                std::cout << "[Unsupported numeric length]";
            }
        } else {
            std::cout << "[Unknown field type]";
        }
        offset += field.length;
    }
    std::cout << std::endl;
    Record record = {};
    return record;
}


