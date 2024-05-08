#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include "common.h"
#include "lexer.cpp"


bool compile(std::string fileName) {
    std::string line;
    std::vector<std::string> data;
    std::ifstream raw;
    raw.open(fileName);
    if (raw.is_open()) {
        while (std::getline(raw, line)) {
            data.push_back(line);
        }
        if (!lexizer(fileName, &data)) return false;
        raw.close();
        return true;
    } else {
        std::cout << "compilation interrupted\n";
        return false;
    }
    return false;
}