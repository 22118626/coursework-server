//
// Created by ener9 on 17/11/2024.
//

#ifndef COURSEWORK_SERVER_DATABASE_H
#define COURSEWORK_SERVER_DATABASE_H

#include "Table.h"

class Database {
public:
    Database();
    void Init();


private:
    std::vector<std::unique_ptr<Table>> tables;


};


#endif //COURSEWORK_SERVER_DATABASE_H
