#include <iostream>
#include <fstream>
#include "compiler.cpp"
#include "common.h"

float VERSION;

int main(int argc, char* argv[]) {
    VERSION = 0.1;
    // std::vector<std::string> args(argv, argv + argc);
    // std::cout << "argc: " << argc << "\n\n"; 
    // for (std::string& arg : args) {
    //     std::cout << "argv: " << arg << "\n";
    // }
    std::ifstream raw;
    raw.open(argv[1]);
    if (raw.is_open()) {
        raw.close();
        if (compile(argv[1]) == true) {
            std::cout << "\nbuild successful" << std::endl;
        }
        else {
            std::cout << "\nbuild failed";
        }
    } else {
        std::cout << "input file not found\n\nbuild failed";
    }
    return 0;
}