//
// Created by ener9 on 17/11/2024.
//

#include "Database.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <openssl/sha.h>


namespace fs = std::filesystem;

Database::Database() {
    Init();
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
    std::cout << "peepeepoopoo" << std::endl;
    nlohmann::json json;
    json = nlohmann::json::parse(jstring);

    for (const auto &table : this->tables) {
        nlohmann::json jsonDataArray;
        std::cout << "pee pee" << table->tableName << std::endl << to_string(json) << std::endl;
        std::cout<<json["tableName"].get<std::string>()<<std::endl;
        if(table->tableName != json["tableName"].get<std::string>()) {
            std::cout<<"TableName: "<<table->tableName<<std::endl;
            continue;
        }
        if(json.contains("data") && json["data"].is_object()) {
            jsonDataArray = json["data"];
        } else continue;
        if (json["mode"] == "search") {
            std::cout << "searching "<<table->tableName<<" with "<<jsonDataArray["field"]<<" "<<jsonDataArray["value"]<<std::endl;
            return table->RecordToJson(table->searchTableByFieldNameAndValue(jsonDataArray["field"], jsonDataArray["value"]));
        }
        if (json["mode"] == "authenticate") {
            std::cout << "authenticating "<<table->tableName<<" with "<<jsonDataArray["field"]<<" "<<jsonDataArray["value"]<<std::endl;
            nlohmann::json val = table->RecordToJson(table->searchTableByFieldNameAndValue(jsonDataArray["field"], jsonDataArray["value"]));
            nlohmann::json val2;
            std::ostringstream oss;
            unsigned char hash[SHA512_DIGEST_LENGTH];
            SHA512(reinterpret_cast<const unsigned char*>(val["HashedPassword"].get<std::string>().data()), val["HashedPassword"].get<std::string>().size(), hash);
            for(int i = 0; i< SHA512_DIGEST_LENGTH; i++) {
                oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
            }
            val2["key"] = oss.str();
            return val2;
        }
        if (json["mode"] == "append") {
            std::cout << "appending "<<table->tableName<<" with "<<jsonDataArray.dump()<<std::endl;
            nlohmann::json result;
            result["code"] = table->appendRecordFromJson(jsonDataArray);
            return result;
        }
    }
}

