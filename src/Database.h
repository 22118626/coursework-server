//
// Created by ener9 on 17/11/2024.
//

#ifndef COURSEWORK_SERVER_DATABASE_H
#define COURSEWORK_SERVER_DATABASE_H

#include <nlohmann/json.hpp>

#include "Table.h"

class Database {
public:
    static std::shared_ptr<Table> loginTable;
    static std::shared_ptr<Database> &GetInstance();
    void operator=(const Database&) = delete;
    void Init();

    int UseTable(nlohmann::json json);
    nlohmann::json parseDatabaseCommand(nlohmann::json json, int index);

private:
    Database();
    static std::shared_ptr<Database> instance;
    std::vector<std::shared_ptr<Table>> tables;
    static nlohmann::json search(nlohmann::json json, Table* table);
    static bool AuthenticateValid(nlohmann::json userData);
    static nlohmann::json AuthenticateUser(nlohmann::json userData);
    nlohmann::json GetTables(nlohmann::json json);
};


#endif //COURSEWORK_SERVER_DATABASE_H
