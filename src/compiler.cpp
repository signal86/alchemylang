#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <string>

class ErrorHandler {
    private:
        struct Error {
            int lineNumber;
            std::string error;
            std::string line;
        };
        std::vector<Error> errors;
        bool problem = false;
        std::string fileName;
        std::string startingText;

    public:
        ErrorHandler(std::string startingTextIn, std::string fileNameIn) {
            startingText = startingTextIn;
            fileName = fileNameIn;
        }

        void addError(int lineNumberIn, std::string errorIn, std::string lineIn) {
            Error error = {
                lineNumberIn,
                errorIn,
                lineIn
            };
            errors.push_back(error);
            problem = true;
        }

        bool catcher() {
            if (problem) {
                for (Error error : errors) {
                    std::cout << startingText << " on " << fileName << ":" << error.lineNumber << " -> \"" << error.line << "\" -> " << error.error << "\n";
                }
                return false;
            } else {
                return true;
            }
        }
};

bool lexer(std::string fileName, std::vector<std::string> *data) {
    ErrorHandler errors("lexizer/syntax error", fileName);
    errors.addError(3, "symbol not found", "example line one");
    errors.addError(6, "segfault", "example line two");
    errors.addError(13, "what the sigma", "example line three");
    return errors.catcher();
    // bool problem = false;
    // std::vector<std::pair<int, std::array<std::string, 2>>> errors;

    // problem = true;
    // append an error @ line 3
    // lineNumber, [error, line]
    // std::array<std::string, 2> exampleError = {"syntax error", "example line"};
    // errors.push_back(std::make_pair(3, exampleError));
    // if (problem) {
    //     for (std::pair<int, std::array<std::string, 2>>& error : errors) {
    //         std::cout << "lexizer/syntax error on \"" << fileName << ":" << std::get<int>(error) << " -> \"" << std::get<std::array<std::string, 2>>(error)[1] << "\" -> " << std::get<std::array<std::string, 2>>(error)[0] << "\n";
    //     }
    //     return false;
    // }
    // return true;
}

bool compile(std::string fileName) {
    std::string line;
    std::vector<std::string> data;
    std::ifstream raw;
    raw.open(fileName);
    if (raw.is_open()) {
        while (std::getline(raw, line)) {
            data.push_back(line);
        }
        bool valid = true;
        valid = lexer(fileName, &data);
        raw.close();
        return valid;
    } else {
        std::cout << "compilation interrupted\n";
        return false;
    }
    return false;
}