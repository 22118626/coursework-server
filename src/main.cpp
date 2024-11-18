#include <iostream>
#include <stdio.h>
#include "./Database.h"


#include "CLIApp.h"

int main(int argc, char* argv[]) {
    Database db = Database::GetInstance();
    db.Init();

    nlohmann::json json;

    CLIApp app;
    app.run();

    return 0;
}
