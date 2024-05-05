#include <iostream>
#include <cstdio>
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
            if (!problem) return true;

            for (Error error : errors) {
                // std::cout << startingText << " on " << fileName << ":" << error.lineNumber << " -> \"" << error.line << "\" -> " << error.error << "\n";
                std::printf(
                    "%s on %s:%d -> \"%s\" -> %s\n",
                    startingText.c_str(), fileName.c_str(), error.lineNumber, error.line.c_str(), error.error.c_str()
                );
            }

            return false;
        }
};

bool lexer(std::string fileName, std::vector<std::string> *data) {
    ErrorHandler errors("lexizer/syntax error", fileName);
    errors.addError(3, "symbol not found", "example line one");
    errors.addError(6, "segfault", "example line two");
    errors.addError(13, "what the sigma", "example line three");

    //

    return errors.catcher();
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