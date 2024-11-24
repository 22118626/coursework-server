//
// Created by ener9 on 30/10/2024.
//

#ifndef COURSEWORK_SERVER_TABLE_H
#define COURSEWORK_SERVER_TABLE_H

#include <string>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "Record.h"
#include "FileManager.h"


class Table {
public:
    std::string tableFilePath;

    explicit Table(const std::string& filePath);
    explicit Table();
    virtual ~Table()= default;
    Table& operator=(const Table&) = default;
    //virtual void addRecord(std::shared_ptr<Record> record);
    //void printRecords() const;

    std::string tableName;

    void setFilePath(std::string path);
    void initializeTable();
    Record searchTableByFieldNameAndValue(const std::string &fieldName, const std::string &fieldValue);
    void debugSearch(const std::string &FieldName, const std::string &FieldValue);
    nlohmann::json_abi_v3_11_3::basic_json<> RecordToJson(Record record);
    Record JsonToRecord(nlohmann::json json);
    int appendRecordFromJson(nlohmann::json json);
    int appendRecord(const Record& record);

private:
    FileManager FM;

    std::vector<FieldData> structureRecord;
    int recordSize = 0;

    void addDataTypeToRecord(Record *record, const std::string &name, int type, int dataLength);
    std::variant<int16_t, uint16_t, int32_t, uint32_t, std::string, bool> readRecord(int index);

};


#endif //COURSEWORK_SERVER_TABLE_H
