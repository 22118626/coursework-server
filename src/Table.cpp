//
// Created by ener9 on 30/10/2024.
//

#include "Table.h"
#include <vector>
#include <algorithm>


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

    int16_t NumOfFields = this->FM.readNextInt16_t();
    std::vector<std::string> fields(NumOfFields,"");
    this->tableName = this->FM.name;
    //std::cout << this->FM.currentPointerPosition() << std::endl;
    /*std::cout << "DataStart: " << this->FM.dataStart << std::endl <<
    "Type: " << this->FM.type << std::endl <<
    "name: " << this->FM.name << std::endl <<
    "numberOfFields: " << NumOfFields << std::endl;*/

    /*LoginID uint32| Username String(50) | HashedPassword String(64Bytes/256bits)*/
    for(int i = 0; i < NumOfFields; i++) {
        FieldData fieldData = {this->FM.readNextString(), this->FM.readNextUint8_t(),this->FM.readNextUint16_t()};
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

/*std::variant<int16_t, uint16_t, int32_t, uint32_t, std::string, bool> Table::searchTableByFieldNameAndValue(
        const std::vector<FieldData>& structure, const std::string& fieldName) {

    // Find the field by name
    auto itterator = std::find_if(structure.begin(), structure.end(), [&](const FieldData& field) {
        return field.name == fieldName;
    });

    // Ensure the field is found
    if (itterator == structure.end()) {
        throw std::runtime_error("Field name not found in structure");
    }

    // Find the index of the field
    int indexToSearch = std::distance(structure.begin(), itterator);
    std::cout << "Found field at index: " << indexToSearch << std::endl;

    try {
        // Read the record of type T at the given index
        auto result = readRecord(indexToSearch);

        return result; // Return the result (which is a std::variant)
    } catch (const std::exception& ex) {
        std::cerr << "Error reading record: " << ex.what() << std::endl;
        throw; // Re-throw the exception to the caller
    }
}*/

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

    // If no matching record is found, throw an exception or return a default record
    throw std::runtime_error("Field value not found in any record");
}

void Table::debugSearch(const std::string& FieldName, const std::string& FieldValue) {
    auto result = searchTableByFieldNameAndValue("Username", "Gilbert");
    std::cout << "result: " << result.data.data() << std::endl;
}


