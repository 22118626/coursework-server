//
// Created by ener9 on 17/11/2024.
//

#ifndef COURSEWORK_SERVER_DATABASE_H
#define COURSEWORK_SERVER_DATABASE_H

#include <nlohmann/json.hpp>

#include "Table.h"

class Database {
public:
    static std::shared_ptr<Database> &GetInstance();
    void operator=(const Database&) = delete;
    void Init();

    int UseTable(nlohmann::json json);
    nlohmann::json parseDatabaseCommand(std::string jstring);

private:
    Database();
    static std::shared_ptr<Database> instance;
    std::vector<std::shared_ptr<Table>> tables;


};


#endif //COURSEWORK_SERVER_DATABASE_H
