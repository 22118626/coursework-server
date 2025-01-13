//
// Created by ener9 on 17/11/2024.
//

#include "Database.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <openssl/sha.h>

std::shared_ptr<Database> Database::instance = nullptr;
std::string hash(std::string string);

namespace fs = std::filesystem;

Database::Database() {
    Init();
}

std::shared_ptr<Database> &Database::GetInstance() {
    if(instance == nullptr) {
        instance = std::make_shared<Database>(Database());
    }
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
        std::cout<<"finished Initializing Database 😁"<<std::endl;
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

    std::cout << "Parsing: \t\t"<< jstring << std::endl;
    nlohmann::json json;
    json = nlohmann::json::parse(jstring);
    std::cout << json.dump(2) << std::endl;

    for (const auto &table : this->tables) {
        nlohmann::json jsonDataArray;
        std::shared_ptr<Table> loginTable;
        jsonDataArray = json["data"];
        std::cout << "dingus" << std::endl;

        for(const std::shared_ptr<Table>& table1 : this->tables) {
            if(table1->tableName == "Login") loginTable = table1;
        }
        std::cout << "dingus2" << std::endl;
        if (json["mode"] == "authenticate") {
            std::cout << "authenticating "<<" with "<<json["data"]["username"]<<" "<<jsonDataArray["password"]<<std::endl;
            nlohmann::json val = loginTable->RecordToJson(loginTable->searchTableByFieldNameAndValue("Username", jsonDataArray["username"]));
            std::cout << val.dump(2) << std::endl;
            nlohmann::json rtrn;
            if(val["Username"] == jsonDataArray["username"] && val["HashedPassword"] == jsonDataArray["password"]) {
                rtrn["privileged"] = val["UserPrivelageFlag"] > 0;
                rtrn["access"] = true;
                rtrn["data"] = val;
                return rtrn;
            }
            rtrn["privileged"] = false;
            rtrn["access"] = false;
            return rtrn;
        }
        std::cout << "dingus3" << std::endl;
        if(json["mode"] == "getTables") {

            std::cout<<0<<std::endl;
            nlohmann::json rtrnjson = nlohmann::json::object();
            rtrnjson["data"] = nlohmann::json::object();
            rtrnjson["data"]["array"] = nlohmann::json::array();
            if (true) { // auth*
                std::cout<<1<<std::endl;
                for(auto &tbl : this->tables)  {
                    std::cout<<2<<std::endl;
                    if(json["authentication"]["UserPrivelageFlag"] >= tbl->permissionLevel) {
                        std::cout<<3<<std::endl;
                        std::cout << tbl->tableName << std::endl;
                        nlohmann::json tablejson =nlohmann::json::object();
                        tablejson["tableName"] = tbl->tableName;
                        tablejson["array"]=nlohmann::json::array();
                        std::cout<<4<<std::endl;

                        for (FieldData i : tbl->structureRecord ) {
                            std::cout<<5<<std::endl;
                            nlohmann::json fieldjson = nlohmann::json::object();
                            fieldjson["name"] = i.name;
                            fieldjson["length"] = i.length;
                            fieldjson["type"] = i.type;
                            fieldjson["isPrimary"] = i.isPrimary;

                            tablejson["array"].push_back(fieldjson);
                        }
                        std::cout<<6<<std::endl;
                        rtrnjson["data"]["array"].push_back(tablejson);


                    }
                }
                rtrnjson["code"] = 0;
                rtrnjson["description"] = "success";
                std::cout << rtrnjson.dump(4) << std::endl;
                return rtrnjson;
            }
            rtrnjson["code"] = 2;
            rtrnjson["description"] = "incorrect auth token";
            return rtrnjson;
        }

        if(table->tableName != json["tableName"].get<std::string>()) {
            std::cout<<"TableName: "<<table->tableName<<std::endl;
            continue;
        }
        if(json.contains("data") && json["data"].is_object()) {
            jsonDataArray = json["data"];
        } else continue;
        if (json["mode"] == "search") {
            std::cout << "tableTYPE?!!:   " << table->permissionLevel << std::endl;
            if(table->permissionLevel != 0) {
                nlohmann::json auth = json["authentication"].get<nlohmann::json>();
                nlohmann::json fromTableUser = loginTable->RecordToJson(loginTable->searchTableByFieldNameAndValue("LoginID", std::to_string(auth["LoginID"].get<uint32_t>())));
                if (auth["HashedPassword"] == fromTableUser["HashedPassword"] && table->permissionLevel <= fromTableUser["UserPrivelageFlag"]) {
                    std::cout << "searching with auth: "<<table->tableName<<" with "<<jsonDataArray["field"]<<" "<<jsonDataArray["value"]<<std::endl;
                    return table->RecordToJson(table->searchTableByFieldNameAndValue(jsonDataArray["field"], jsonDataArray["value"]));
                }else {
                    std::cout << "Failed auth" << std::endl;
                    nlohmann::json rtrn;
                    rtrn["code"] = -3;
                    rtrn["description"] = "Failed authentication (Accessing Higher Level requirement Table)";
                    return rtrn;
                }

            }else{
                std::cout << "searching: "<<table->tableName<<" with "<<jsonDataArray["field"]<<" "<<jsonDataArray["value"]<<std::endl;
                return table->RecordToJson(table->searchTableByFieldNameAndValue(jsonDataArray["field"], jsonDataArray["value"]));
            }
        }
        if (json["mode"] == "append") {
            std::cout << "appending "<<table->tableName<<"("+json["tableName"].get<std::string>()+") with "<<jsonDataArray.dump()<<std::endl;
            nlohmann::json result;
            result["code"] = table->appendRecordFromJson(jsonDataArray);
            return result;
        }
        if (json["mode"] == "remove") {
            std::cout << "REMOVING: " << json["data"].dump() <<std::endl;

        }
        std::cout << "dingus4" << std::endl;
    }
    json["code"] = -1;
    json["description"] = "mode not found";
    return json;

}

std::string hash(std::string string) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const unsigned char*>(string.data()), string.size(), hash);
    std::ostringstream oss;
    for(int i = 0; i< SHA256_DIGEST_LENGTH; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}

