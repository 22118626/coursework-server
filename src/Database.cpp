//
// Created by ener9 on 17/11/2024.
//

#include "Database.h"
#include <filesystem>


namespace fs = std::filesystem;

Database::Database() {

}
void Database::Init() {

    std::string path = "../tables";

    try{
        for(const auto &entry : fs::directory_iterator(path)) {
            if(entry.is_regular_file() && entry.path().extension().string() == ".bdb") {
                std::cout << "path: " << entry.path() << std::endl;
                this->tables.push_back(std::make_unique<Table>(entry.path().string()));
                this->tables.at(this->tables.size()-1).get()->initializeTable();
            }
        }
        for(auto &table : this->tables) {
            std::cout << "TableName: " << table->tableName << std::endl;
        }
    }catch (const fs::filesystem_error &e) {std::cerr << "error in Database.cpp/Database::int(): " << e.what() << e.code() << std::endl;}

}
