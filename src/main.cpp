#include <iostream>
#include <stdio.h>


#include "CLIApp.h"

int main(int argc, char* argv[]) {
    std::cout << "String0" << std::endl;
    std::cout << "TestString" << "\r\b" << "string2" << std::endl;
    CLIApp app;
    app.run();

    return 0;
}
