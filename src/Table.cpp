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

// 1 indexed
Record Table::readRecord(int index) {
    this->FM.setPointerLoc(this->FM.dataStart + index * this->recordSize);
    auto vec = this->FM.readBytes(recordSize);
    for (auto byte : vec) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::endl;
    Record record = {};
    return record;
}

/*void Table::addDataTypeToRecord(Record* record, const std::string &name, int type, int dataLength) {
    switch (type) {
        case 1:
            record->record.push_back(std::make_shared<Int16Field>(name, 0x00));
            recordFieldType.push_back(1);
            break;
        case 2:
            record->record.push_back(std::make_shared<Int32Field>(name, 0x00));
            recordFieldType.push_back(2);
            break;
        case 3:
            record->record.push_back(std::make_shared<StringField>(name, "", dataLength));
            recordFieldType.push_back(3);
            break;
        case 4:
            record->record.push_back(std::make_shared<ReferenceField>(name, "", 0x00));
            recordFieldType.push_back(4);
            break;
        case 5:
            record->record.push_back(std::make_shared<BoolField>(name, false));
            recordFieldType.push_back(4);
            break;
        default:
            std::cout <<"hip hip hooray... disaster has been prevented?" << std::endl;
    }
    return;
}*/

