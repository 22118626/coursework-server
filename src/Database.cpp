//
// Created by ener9 on 17/11/2024.
//

#include "Database.h"
#include <filesystem>
#include <nlohmann/json.hpp>


namespace fs = std::filesystem;

Database::Database() {
}


Database& Database::GetInstance() {
    static Database instance;
    return instance;
}

void Database::Init() {

    std::string path = "../tables";

    try{
        for(const auto &entry : fs::directory_iterator(path)) {
            if(entry.is_regular_file() && entry.path().extension().string() == ".bdb") {
                std::cout << "path: " << entry.path() << std::endl;
                this->tables.push_back(std::make_shared<Table>(entry.path().string()));
                this->tables.at(this->tables.size()-1).get()->initializeTable();
            }
        }
        for(const auto &table : this->tables) {
            std::cout << "TableName: " << table->tableName << std::endl;
        }
    }catch (const fs::filesystem_error &e) {std::cerr << "error in Database.cpp/Database::int(): " << e.what() << e.code() << std::endl;}

}

int Database::UseTable(nlohmann::json json) {
    for(const auto &table : this->tables) {
        if(table->tableName == json["tableName"].get<std::string>()) {
            continue;
        }
    }
    return 0;
}
nlohmann::json Database::parseDatabaseCommand(std::string jstring) {
    nlohmann::json json;
    json = nlohmann::json::parse(jstring);

    for (const auto &table : this->tables) {
        nlohmann::json jsonDataArray;
        std::cout << "pee pee" << table->tableName << std::endl << to_string(json) << std::endl;
        if(table->tableName != json["tableName"].get<std::string>()) {
            std::cout<<"TableName: "<<table->tableName<<std::endl;
            continue;
        }
        if(json.contains("data") && json["data"].is_object()) {
            jsonDataArray = json["data"];
        }
        if (json["mode"] == "search") {
            table->searchTableByFieldNameAndValue(jsonDataArray["field"], jsonDataArray["value"]);
        }

    }
}

