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
                std::cout << "path: " << entry.path() << std::endl;
                this->tables.push_back(std::make_shared<Table>(entry.path().string()));
                this->tables.at(this->tables.size()-1).get()->initializeTable();
            }
        }
        for(const auto &table : this->tables) {
            std::cout << "TableName: " << table->tableName << std::endl;
            if(table->tableName == "Login") {
                Database::loginTable = table;
                std::cout << "saved loginTable >> " << Database::loginTable << "\t" << Database::loginTable->tableName << std::endl;
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

    std::cout << "jsondata " << jsonDataArray.dump(4) << std::endl;
    std::cout << "table: " << table  << table->tableName << std::endl;
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
        std::cout << "appending "<<table->tableName<<"("+json["tableName"].get<std::string>()+") with "<<jsonDataArray.dump()<<std::endl;
        nlohmann::json result;
        result["code"] = table->appendRecordFromJson(jsonDataArray);
        return result;
    }
    if (json["mode"] == "remove") {
        std::cout << "REMOVING: " << json["data"].dump() <<std::endl;
        Record record = table->JsonToRecord(jsonDataArray);
        nlohmann::json result;
        result["code"]= table->removeRecordFromTable(record);
        result["description"]= result==0 ? "success!" : "failed :(";
        return result;
    }
    if (json["mode"] == "modify") {
        std::cout << "MODIFYING: " << json["data"].dump() <<std::endl;
        Record record = table->JsonToRecord(jsonDataArray);
        nlohmann::json result;
        result["code"]= table->modifyRecordFromOldRecord(record);
        result["description"]= result["code"]==0 ? "success!" : "failed :(";
        return result;
    }
}
nlohmann::json Database::search (nlohmann::json json, Table *table) {
    if(table->permissionLevel != 0) {
        nlohmann::json auth = json["authentication"].get<nlohmann::json>();
        nlohmann::json fromTableUser = Database::loginTable->RecordToJson(Database::loginTable->searchTableByFieldNameAndValue("LoginID", std::to_string(auth["LoginID"].get<uint32_t>())));
        if (auth["HashedPassword"] == fromTableUser["HashedPassword"] && table->permissionLevel <= fromTableUser["UserPrivelageFlag"]) {
            std::cout << "searching with auth: "<<table->tableName<<" with "<<json["data"]["field"]<<" "<<json["data"]["value"]<<std::endl;
            nlohmann::json rtrn;
            rtrn["data"] = table->RecordToJson(table->searchTableByFieldNameAndValue(json["data"]["field"], json["data"]["value"]));
            rtrn["code"] = 0;
            rtrn["description"] = "Success";
            return rtrn;
        }else {
            std::cout << "Not found" << std::endl;
            nlohmann::json rtrn;
            rtrn["code"] = -4;
            rtrn["description"] = "Not found";
            return rtrn;
        }
    }
    else{
        std::cout << "Failed auth" << std::endl;
        nlohmann::json rtrn;
        rtrn["code"] = -3;
        rtrn["description"] = "Failed authentication (Accessing Higher Level requirement Table)";
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
    std::cout << "authenticating "<<" with "<<jsonDataArray["username"]<<" "<<jsonDataArray["password"]<<std::endl;
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
    std::cout << 1 << std::endl;
    nlohmann::json rtrnjson = nlohmann::json::object();
    rtrnjson["data"] = nlohmann::json::object();
    rtrnjson["data"]["array"] = nlohmann::json::array();
    // auth*
    std::cout << 2 << std::endl;
    if (!Database::AuthenticateValid(json["authentication"])) {
        nlohmann::json returnjson;
        returnjson["code"] = 2;
        returnjson["description"] = "user not valid";
        return returnjson;
    }
    std::cout << 3 << std::endl;
    for(auto &tbl : this->tables)  {
        // if statement validates that the users permission is higher or equal to the table that is being checked
        if(json["authentication"]["UserPrivelageFlag"] >= tbl->permissionLevel) {
            nlohmann::json tablejson =nlohmann::json::object();
            tablejson["tableName"] = tbl->tableName;
            tablejson["array"]=nlohmann::json::array();
            // gives all table data for the client to interpret

            std::cout << 4 << std::endl;
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
    std::cout << 5 << std::endl;
    rtrnjson["code"] = 0;
    rtrnjson["description"] = "success";
    std::cout << rtrnjson.dump(4) << std::endl;
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

