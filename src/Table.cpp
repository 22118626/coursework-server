//
// Created by 22118626 on 30/10/2024.
//

#include "Table.h"
#include <utility>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>



Table::Table(const std::string &filePath) :tableFilePath(filePath), FM(filePath,0) {
    std::cout << this->tableFilePath << std::endl;
}
Table::Table() :FM("",0) {};

void Table::setFilePath(std::string path) {
    this->tableFilePath = path;
     FM.changeFilePath(path);
}

void Table::initializeTable() {
    //this opens the file. if the OS does not allow for opening the file then it returns without executing any further instructions
    if (!this->FM.openFile(this->tableFilePath)) {
        std::cout << "oppening file (" << this->tableFilePath << ") failed" << std::endl;
        return;
    }

    // this ensures that the file is read from the 1st byte
    // and will read all metadata information sequentially and initialize it into its respective variables.
    this->FM.setPointerLoc(0);
    this->FM.readHeader();
    this->lastPrimaryKeyIndexPointer = this->FM.currentPointerPosition();
    this->lastPrimaryKeyIndex = this->FM.readNextInt32_t();

    int16_t NumOfFields = this->FM.readNextInt16_t();
    std::vector<std::string> fields(NumOfFields,"");
    this->tableName = this->FM.name;
    this->permissionLevel = this->FM.permissionLevel;

    /*LoginID uint32| Username String(50) | HashedPassword String(64Bytes/256bits)*/
    // this itterates though NumOfFields amount of fields and the generic information required to store the data
    // this includes the name of the field and the datatype held inside of it
    for(int i = 0; i < NumOfFields; i++) {
        std::string str = this->FM.readNextString();
        uint8_t type = this->FM.readNextUint8_t();
        uint16_t length = this->FM.readNextUint16_t();
        if (type==4) {
            std::string fieldTableName = this->FM.readNextString();
            uint16_t tableNameLength = this->FM.readNextUint16_t();
            FieldData fieldData = {str, type, length, (i==0), fieldTableName, tableNameLength};
            structureRecord.push_back(fieldData);
        }else {
            FieldData fieldData = {str, type, length, (i==0)};
            structureRecord.push_back(fieldData);
        }
    }
    // this code increments the value of this->recordSize to represent the length of each record in the table
    this->recordSize = 0;
    for ( auto i : structureRecord) {
        //std::cout << "name: " << i.name << "\tlength: " << i.length << std::endl;
        this->recordSize += i.length;
    }
    std::cout << "Finished reading record metadata. RecordSize: " << this->recordSize << std::endl;
}

// this code was initially intended to be used for reading a record at a given index of the record, however it is
// deprecated at it could be unreliable due to the fact that index of a record could vary if all records have a
// differing size
/// @deprecated
std::variant<int16_t, uint16_t, int32_t, uint32_t, std::string, bool> Table::readRecord(int index) {
    this->FM.setPointerLoc(this->FM.dataStart + index * this->recordSize);
    auto vec = this->FM.readBytes(recordSize);

    int offset = 0;
    for (auto& field : this->structureRecord) {

        if (field.type == 3) {
            auto value = std::string(reinterpret_cast<const char*>(vec.data()) + offset, field.length);
            return value;
        } else if (field.type == 4 || field.type == 2 || field.type == 1) {
            if (field.length == 4) {
                int32_t value = *reinterpret_cast<const int32_t*>(vec.data() + offset);
                return value;
            } else if (field.length == 2) {
                int16_t value = *reinterpret_cast<const int16_t*>(vec.data() + offset);
                return value;
            }
        } else {
            std::cerr << "[Unknown field type]" << std::endl;
            throw std::invalid_argument("Unknown field type");
        }

        offset += field.length;
    }

    throw std::runtime_error("Record parsing failed");
}


Record Table::searchTableByFieldNameAndValue(const std::string& fieldName, const std::string& fieldValue) {
    try{
        // initializes variables and ensures that the reading pointer is set to the beginning of the database
        size_t totalRecords = (this->FM.getFileSize() - this->FM.dataStart) / recordSize;
        this->FM.setPointerLoc(this->FM.dataStart);

        for (size_t index = 0; index < totalRecords; ++index) {
            // this will read "recordSize" length of bytes from the pointer's current position and save it to recordBytes
            std::vector<uint8_t> recordBytes = this->FM.readBytes(recordSize);

            Record record;
            record.data = recordBytes;

            size_t offset = 0;
            bool found = false;

            // compares the structure of the record to the data retrieved and parses it into manageable chunks
            // also changes the bytes array into its respective datatype
            // (E.g. 2 bytes of value 0x01 0xA1 can be constructed back into 16bit integer of value 41217 in Little Endian)
            for (const FieldData &field : this->structureRecord) {
                if (field.name == fieldName) {
                    if (field.type == 3) {
                        std::string s = std::string(reinterpret_cast<const char*>(&record.data[offset]), field.length);
                        s.erase(std::ranges::remove(s, '\0').begin(), s.end());
                        if (s == fieldValue) {
                            found = true;
                        }
                    }
                    else if (field.type == 4 || field.type == 2 || field.type == 1) {
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
                    this->recentIndex = index;
                    return record;
                }
            }
        }
        // if the record has not enough records or no record was found return an empty record with an ID of 0
        Record rec;
        rec.data.push_back(0x00); // no id of 0
        return rec;
    } catch (const nlohmann::json_abi_v3_11_3::detail::type_error& e) {
        std::cerr << "Error parsing JSON or handling UTF-8 data: " << e.what() << std::endl;
        // Return an empty record or a record indicating failure
        Record rec;
        rec.data.push_back(0x01);
        return rec;
    } catch (const std::exception &e) {
        std::cerr << "Error parsing JSON or handling UTF-8 data: " << e.what() << std::endl;
        Record rec;
        rec.data.push_back(0x02);
        return rec;
    }
}

/// wrapper functions for easier programming
int Table::appendRecordFromJson(nlohmann::json json) {
    return appendRecord(JsonToRecord(std::move(json)));
}
int Table::appendRecord(const Record& record) {
    setPrimaryKeyIndex(this->lastPrimaryKeyIndex+1);
    return FM.appendAtTheEnd(record.data);
}
void Table::setPrimaryKeyIndex(int keyIndex) {
    this->lastPrimaryKeyIndex = keyIndex;
    FM.writeAt<uint32_t>(keyIndex, lastPrimaryKeyIndexPointer);
}

int Table::modifyRecordFromOldRecord(Record newRec) {
    for(auto field : this->structureRecord) {
        if(field.isPrimary) {
            this->PrimaryField = field;
        }
    }
    fpos_t position = getPointerOfRecord(searchTableByFieldNameAndValue(this->PrimaryField.name,
            std::to_string(this->RecordToJson(newRec)[this->PrimaryField.name].get<uint32_t>())));
    return FM.modifyAtPointer(position, newRec.data);
}
// this code effectively removes a record from the database by shifting all the succeeding records over the deleting record, overwriting data and shrinking the file size
int Table::removeRecordFromTable(Record record) {
    fpos_t cpp = getPointerOfRecord(record);
    if(cpp!=0) {
        int val = FM.ShiftDataFromfpos(cpp+recordSize,-recordSize);
        setPrimaryKeyIndex(this->lastPrimaryKeyIndex-1);
        return val;
    }
    return 1;
}
// itterates through all records if the record given and the record read from the file match then the pointer of the records is returned
fpos_t Table::getPointerOfRecord(Record record) {
    FM.setPointerLoc(FM.dataStart);
    for (int i = 1; i < (FM.getFileSize() - FM.dataStart /
                                            this->recordSize); i++) { //i = 1 may be incorrect and miss out the last record come back later if issues
        fpos_t CPP = FM.currentPointerPosition();
        std::vector<uint8_t> rec = FM.readBytes(this->recordSize);
        if (record.data == rec) {
            return CPP;
        }
    }
    return 0;
}

// itterates through the structure of the record given by the header of the fiel and matches the names of the fields with th names of json entries
nlohmann::json Table::RecordToJson(Record record) {
    int offset = 0;
    nlohmann::json json;
    for(const FieldData &field : this->structureRecord) {
        // conversion for number types
        if(field.type==2 || field.type==1){
            if(field.length == 2) {
                json[field.name] = record.getFieldData<uint16_t>(offset);
            }
            if(field.length == 4) {
                json[field.name] = record.getFieldData<uint32_t>(offset);
            }
        }
        //conversions for string types
        else if(field.type==3) {
            std::string s = std::string(reinterpret_cast<const char*>(&record.data[offset]), field.length);
            s.erase(std::ranges::remove(s, '\0').begin(), s.end());
            json[field.name]=s;
        }else if(field.type==4) {
            nlohmann::json obj = nlohmann::json::object();
            obj["value"] = record.getFieldData<uint32_t>(offset);
            obj["TableName"] = field.tableName;
            json[field.name] = obj;
//            std::string s = std::string(reinterpret_cast<const char*>(&record.data[offset]), field.length - sizeof(uint32_t));
//            s.erase(std::ranges::remove(s, '\0').begin(), s.end());
//            nlohmann::json referenceObject;
//            referenceObject["tableName"] = s;
//            referenceObject["value"] = record.getFieldData<uint32_t>(offset + field.length - sizeof(uint32_t));
//            json[field.name]=referenceObject;
        }
        offset+= field.length;
    }
    return json;
}
// similar to Table::RecordToJson this method does it backwards, it itterates through the structure and appends the value from the json to the record within the data length and returns the array of bytes
Record Table::JsonToRecord(nlohmann::json json) {
    int offset = 0;
    Record record;
    for(const FieldData &field : this->structureRecord) {
        if(field.isPrimary) {
            record = record.appendField<uint32_t>(&record,json[field.name].get<uint32_t>(), field.length);
            continue;
        }
        // integer types
        if(field.type==2 || field.type==1){
            if(field.length == 2) {
                record = record.appendField(&record, json[field.name].get<uint16_t>(), field.length);
            }
            if(field.length == 4) {
                record = record.appendField(&record, json[field.name].get<uint32_t>(), field.length);
            }
        }
        //string types
        else if(field.type==3) {
            std::string s;
            if (field.type==3) s = json[field.name].get<std::string>();
            else if(field.type == 4 && json[field.name].is_object()) s = json[field.name].get<nlohmann::json>()["tableName"].get<std::string>();
            s.resize(field.length, 0x00);
            record = record.appendStringField(&record, s, field.length); //-sizeof(uint32_t));
            if(field.type == 4) {
                record = record.appendField(&record, json[field.name].get<nlohmann::json>()["value"].get<uint32_t>(), sizeof(uint32_t));
            }
        }else if(field.type==4) {
            record = record.appendField(&record, json[field.name]["value"].get<uint32_t>(), field.length);
        }
        offset+= field.length;
    }
    return record;
}


// this method is called from the CLI commands and prints the data to the console in bytes
void Table::debugSearch(const std::string& FieldName, const std::string& FieldValue) {
    auto result = searchTableByFieldNameAndValue("Username", "Gilbert");
    std::cout<<"result: " << std::endl;for (const auto& byte : result.data) {std::cout<< std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";}std::cout <<std::endl;
}


