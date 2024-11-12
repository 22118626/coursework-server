//
// Created by ener9 on 30/10/2024.
//

#include "Table.h"
#include <vector>


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

    /*LoginID uint32| Username String(50) | HashedPassword String(128)*/
    for(int i = 0; i < NumOfFields; i++) {
        std::string fieldName = this->FM.readNextString();
        uint8_t fieldType = this->FM.readNextUint8_t();
        uint16_t dataLength = this->FM.readNextUint16_t();
        addDataTypeToRecord(&this->structureRecord, fieldName, fieldType, dataLength);
        std::cout << this->FM.currentPointerPosition() << std::endl;
    }
    std::cout << "Finished reading record metadata: " << std::endl <<
    Record::printRecords(this->structureRecord,this->recordFieldType) << std::endl;
    std::cout << std::to_string(structureRecord.returnvalueFromindex<Int32Field >(structureRecord.fieldName("LoginID")).value);
}


void Table::addRecord(std::shared_ptr<Record> record) {

}

void Table::addDataTypeToRecord(Record* record, const std::string &name, int type, int dataLength) {
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
}

