#include <iostream>


struct Variable {
    bool global = false;
    bool constant = false;
    bool varUsed = false;
    bool signal = false;
    bool initialized = false;
    std::string variableName = "";
    std::string value = "";
    std::string scope = "";
};