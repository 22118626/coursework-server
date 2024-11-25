//
// Created by ener9 on 30/10/2024.
//

#include "Table.h"
#include <utility>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>


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
}
Table::Table() :FM("",0) {};

void Table::setFilePath(std::string path) {
    this->tableFilePath = path;
     FM.changeFilePath(path);
}

void Table::initializeTable() {

    if (!this->FM.openFile(this->tableFilePath)) {
        std::cout << "oppening file (" << this->tableFilePath << ") failed" << std::endl;
        return;
    }
    this->FM.setPointerLoc(0);
    this->FM.readHeader();
    this->lastPrimaryKeyIndexPointer = this->FM.currentPointerPosition();
    std::cout << this->lastPrimaryKeyIndexPointer << std::endl;
    this->lastPrimaryKeyIndex = this->FM.readNextInt32_t();
    std::cout << this->lastPrimaryKeyIndex << std::endl;

    int16_t NumOfFields = this->FM.readNextInt16_t();
    std::vector<std::string> fields(NumOfFields,"");
    this->tableName = this->FM.name;

    /*LoginID uint32| Username String(50) | HashedPassword String(64Bytes/256bits)*/
    for(int i = 0; i < NumOfFields; i++) {
        FieldData fieldData = {this->FM.readNextString(), this->FM.readNextUint8_t(),this->FM.readNextUint16_t(), (i==0)};
        structureRecord.push_back(fieldData);
        //std::cout << this->FM.currentPointerPosition() << std::endl;
    }
    this->recordSize = 0;
    for ( auto i : structureRecord) {
        //std::cout << "name: " << i.name << "\tlength: " << i.length << std::endl;
        this->recordSize += i.length;
    }
    std::cout << "Finished reading record metadata. RecordSize: " << this->recordSize << std::endl;
}


std::variant<int16_t, uint16_t, int32_t, uint32_t, std::string, bool> Table::readRecord(int index) {
    this->FM.setPointerLoc(this->FM.dataStart + index * this->recordSize);
    auto vec = this->FM.readBytes(recordSize);

    int offset = 0;
    for (auto& field : this->structureRecord) {
        std::cout << "offset: " << offset << std::endl;

        if (field.type == 4 || field.type == 3) {
            auto value = std::string(reinterpret_cast<const char*>(vec.data()) + offset, field.length);
            std::cout << field.name << ": " << value << std::endl;
            return value;
        } else if (field.type == 2 || field.type == 1) {
            if (field.length == 4) {
                int32_t value = *reinterpret_cast<const int32_t*>(vec.data() + offset);
                std::cout << field.name << ": " << std::dec << value << std::endl;
                return value;
            } else if (field.length == 2) {
                int16_t value = *reinterpret_cast<const int16_t*>(vec.data() + offset);
                std::cout << field.name << ": " << std::dec << value << std::endl;
                return value;
            }
        } else {
            std::cout << "[Unknown field type]" << std::endl;
            throw std::invalid_argument("Unknown field type");
        }

        offset += field.length;
    }

    throw std::runtime_error("Record parsing failed");
}


Record Table::searchTableByFieldNameAndValue(const std::string& fieldName, const std::string& fieldValue) {
    std::cout << "Searching for: Field(" << fieldName << ") value(" << fieldValue << ")" << std::endl;
    size_t totalRecords = (this->FM.getFileSize() - this->FM.dataStart) / recordSize;
    this->FM.setPointerLoc(this->FM.dataStart);

    for (size_t index = 0; index < totalRecords; ++index) {
        std::vector<uint8_t> recordBytes = this->FM.readBytes(recordSize);

        Record record;
        record.data = recordBytes;

        size_t offset = 0;
        bool found = false;

        for (const FieldData &field : this->structureRecord) {
            if (field.name == fieldName) {
                if (field.type == 4 || field.type == 3) {
                    std::string s = std::string(reinterpret_cast<const char*>(&record.data[offset]), field.length);
                    s.erase(std::ranges::remove(s, '\0').begin(), s.end());
                    std::cout << s << std::endl;
                    if (s == fieldValue) {
                        found = true;
                    }
                }
                else if (field.type == 2 || field.type == 1) {
                    if (field.length == 4) {  // int32_t
                        int32_t fieldData = *reinterpret_cast<const int32_t*>(&record.data[offset]);
                        if (std::to_string(fieldData) == fieldValue) {
                            found = true;
                        }
                    } else if (field.length == 2) {  // int16_t
                        int16_t fieldData = *reinterpret_cast<const int16_t*>(&record.data[offset]);
                        if (std::to_string(fieldData) == fieldValue) {
                            found = true;
                        }
                    }
                }
            }
            offset += field.length;

            if (found) {
                std::cout<<"found record: " << std::endl;for (const auto& byte : record.data) {std::cout<< std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";}std::cout <<std::endl;
                return record;
            }
        }
    }
    Record rec;
    rec.data.push_back(0x0000); // no id of 0
    return rec;
}
int Table::appendRecordFromJson(nlohmann::json json) {
    return appendRecord(JsonToRecord(std::move(json)));
}
int Table::appendRecord(const Record& record) {
    lastPrimaryKeyIndex++;
    FM.writeAt<uint32_t>(this->lastPrimaryKeyIndex, lastPrimaryKeyIndexPointer);
    return FM.appendAtTheEnd(record.data);
}


nlohmann::json Table::RecordToJson(Record record) {
    int offset = 0;
    nlohmann::json json;
    for(const FieldData &field : this->structureRecord) {
        if(field.type==2 || field.type==1){
            if(field.length == 2) {
                json[field.name] = record.getFieldData<uint16_t>(offset);
            }
            if(field.length == 4) {
                json[field.name] = record.getFieldData<uint32_t>(offset);
            }
        }
        else if(field.type==3||field.type==4) {
            std::string s = std::string(reinterpret_cast<const char*>(&record.data[offset]), field.length);
            s.erase(std::ranges::remove(s, '\0').begin(), s.end());
            json[field.name]=s;
        }
        offset+= field.length;
    }
    return json;
}
Record Table::JsonToRecord(nlohmann::json json) {
    std::cout << "current Last Index: " << this->lastPrimaryKeyIndex << std::endl;
    int offset = 0;
    Record record;
    for(const FieldData &field : this->structureRecord) {
        std::cout << field.name << std::endl;
        if(field.isPrimary) {
            record = record.appendField<uint32_t>(&record,lastPrimaryKeyIndex+1, field.length);
            continue;
        }
        if(field.type==2 || field.type==1){
            if(field.length == 2) {
                record = record.appendField(&record, json[field.name].get<uint16_t>(), field.length);
            }
            if(field.length == 4) {
                record = record.appendField(&record, json[field.name].get<uint32_t>(), field.length);
            }
        }
        else if(field.type==3||field.type==4) {
            std::string s = json[field.name].get<std::string>();
            s.resize(field.length, 0x00);
            record = record.appendStringField(&record, s, field.length);
        }
        offset+= field.length;
    }
    std::cout<<"found record: " << std::endl;for (const auto& byte : record.data) {std::cout<< std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";}std::cout <<std::endl;
    return record;
}



void Table::debugSearch(const std::string& FieldName, const std::string& FieldValue) {
    auto result = searchTableByFieldNameAndValue("Username", "Gilbert");
    std::cout << "result: " << result.data.data() << std::endl;
}


