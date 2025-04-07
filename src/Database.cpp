//
// Created by ener9 on 17/11/2024.
//

#include "Database.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <openssl/sha.h>

std::shared_ptr<Table> Database::loginTable = nullptr;
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

// initializes the database object and loads all .bdb files in the .\tables\ directory as individual Table onjects
void Database::Init() {

    std::string path = "./tables";

    try{
        for(const auto &entry : fs::directory_iterator(path)) {
            if(entry.is_regular_file() && entry.path().extension().string() == ".bdb" && !(entry.path().filename().string().starts_with(".") || entry.path().filename().string().starts_with("~"))) { // not a directory AND contains ".bdb" extension AND file name does not start with "."OR"~"
                this->tables.push_back(std::make_shared<Table>(entry.path().string()));
                this->tables.at(this->tables.size()-1).get()->initializeTable();
            }
        }
        for(const auto &table : this->tables) {
            std::cout << "initializing TableName: " << table->tableName << std::endl;
            if(table->tableName == "Login") {
                Database::loginTable = table;
            }
        }
        std::cout<<"finished Initializing Database 😁"<<std::endl;
    }catch (const fs::filesystem_error &e) {std::cerr << "error in Database.cpp/Database::int(): " << e.what() << e.code() << std::endl;}

}

// interprests what the client has sent and executes the commands
nlohmann::json Database::parseDatabaseCommand(nlohmann::json json, int index) {

    if (index >= this->tables.size()) {
        json["code"] = -1;
        json["description"] = "mode not found";
        return json;
    }
    // informs the user what command is being parsed and about to be executed

    std::shared_ptr<Table> table = this->tables[index];
    nlohmann::json jsonDataArray = (json.contains("data") && json["data"].is_object()) ? json["data"] : nlohmann::json() ;
    /// non-table specific commands
    if(json["mode"] == "authenticate") return Database::AuthenticateUser(jsonDataArray);
    if(json["mode"] == "getTables") return this->GetTables(json);


    // recursive call if not the correct name
    if(table->tableName != json["tableName"].get<std::string>()) {
        return this->parseDatabaseCommand(json, index+1);
    }
    if (json["mode"] == "search") {
        return  search(json, table.get());
    }
    if (json["mode"] == "append") {
        nlohmann::json result;
        result["code"] = table->appendRecordFromJson(jsonDataArray);
        return result;
    }
    if (json["mode"] == "remove") {
        Record record = table->JsonToRecord(jsonDataArray);
        nlohmann::json result;
        result["code"]= table->removeRecordFromTable(record);
        result["description"]= result==0 ? "success!" : "failed :(";
        return result;
    }
    if (json["mode"] == "modify") {
        Record record = table->JsonToRecord(jsonDataArray);
        nlohmann::json result;
        result["code"]= table->modifyRecordFromOldRecord(record);
        result["description"]= result["code"]==0 ? "success!" : "failed :(";
        return result;
    }
}
nlohmann::json Database::search (nlohmann::json json, Table *table) {
    try {
        if(table->permissionLevel != 0) {
            nlohmann::json auth = json["authentication"].get<nlohmann::json>();
            nlohmann::json fromTableUser = Database::loginTable->RecordToJson(Database::loginTable->searchTableByFieldNameAndValue("LoginID", std::to_string(auth["LoginID"].get<uint32_t>())));
            if (auth["HashedPassword"] == fromTableUser["HashedPassword"] && table->permissionLevel <= fromTableUser["UserPrivelageFlag"]) {
                Record searchRecord = table->searchTableByFieldNameAndValue(json["data"]["field"], json["data"]["value"]);
                if (searchRecord.data.size() > 1 ) {
                    nlohmann::json rtrn;
                    rtrn["data"] = table->RecordToJson(searchRecord);
                    rtrn["code"] = 0;
                    rtrn["description"] = "Success";
                    return rtrn;
                }
                nlohmann::json rtrn;
                rtrn["code"] = -4;
                rtrn["description"] = "Not found";
                return rtrn;
            }
        }
        else{
            nlohmann::json rtrn;
            rtrn["code"] = -3;
            rtrn["description"] = "Failed authentication (Accessing Higher Level requirement Table)";
            return rtrn;
        }
    }catch (const std::exception &e) {
        nlohmann::json rtrn;
        rtrn["code"] = -5;
        rtrn["description"] = "error occured"+std::string(e.what());
        return rtrn;
    }
}

bool Database::AuthenticateValid(nlohmann::json userData) {
    nlohmann::json val = Database::loginTable->RecordToJson(Database::loginTable->searchTableByFieldNameAndValue("Username", userData["Username"]));
    if(val["Username"] == userData["Username"] && val["HashedPassword"] == userData["HashedPassword"]) {
        return true;
    }
    return false;
}

nlohmann::json Database::AuthenticateUser(nlohmann::json jsonDataArray) {
    nlohmann::json val = Database::loginTable->RecordToJson(Database::loginTable->searchTableByFieldNameAndValue("Username", jsonDataArray["username"]));
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

nlohmann::json Database::GetTables(nlohmann::json json) {
    nlohmann::json rtrnjson = nlohmann::json::object();
    rtrnjson["data"] = nlohmann::json::object();
    rtrnjson["data"]["array"] = nlohmann::json::array();
    // auth*
    if (!Database::AuthenticateValid(json["authentication"])) {
        nlohmann::json returnjson;
        returnjson["code"] = 2;
        returnjson["description"] = "user not valid";
        return returnjson;
    }
    for(auto &tbl : this->tables)  {
        // if statement validates that the users permission is higher or equal to the table that is being checked
        if(json["authentication"]["UserPrivelageFlag"] >= tbl->permissionLevel) {
            nlohmann::json tablejson =nlohmann::json::object();
            tablejson["tableName"] = tbl->tableName;
            tablejson["array"]=nlohmann::json::array();
            // gives all table data for the client to interpret

            for (FieldData i : tbl->structureRecord ) {
                nlohmann::json fieldjson = nlohmann::json::object();
                fieldjson["name"] = i.name;
                fieldjson["length"] = i.length;
                fieldjson["type"] = i.type;
                fieldjson["isPrimary"] = i.isPrimary;

                tablejson["array"].push_back(fieldjson);
            }
            rtrnjson["data"]["array"].push_back(tablejson);


        }
    }
    rtrnjson["code"] = 0;
    rtrnjson["description"] = "success";
    std::cout << "get tables : " << rtrnjson.dump() << std::endl;
    return rtrnjson;
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

