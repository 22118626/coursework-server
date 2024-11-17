#include <iostream>
#include <stdio.h>
#include "./Database.h"


#include "CLIApp.h"

int main(int argc, char* argv[]) {
    Database db;
    db.Init();
    CLIApp app;
    app.run();

    return 0;
}
