#include <iostream>
#include <cstdio>
#include <vector>
#include <array>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>

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
                std::printf(
                    "%s on %s:%d -> \"%s\" -> %s\n",
                    startingText.c_str(), fileName.c_str(), error.lineNumber, error.line.c_str(), error.error.c_str()
                );
            }

            return false;
        }
};

bool lexer(std::string fileName, std::vector<std::string> *data) {
    std::vector<std::string> dataCopy = *data;
    ErrorHandler errors("lexizer/syntax error", fileName);
    // errors.addError(3, "symbol not found", "example line one");

    std::string architecture = "none";
    for (auto i = dataCopy.begin(); i != dataCopy.end(); i++) {

        // beginning whitespace trim
        dataCopy[std::distance(dataCopy.begin(), i)] =
            (dataCopy[std::distance(dataCopy.begin(), i)].find_first_not_of(" \n\r\t\f\v") == std::string::npos) ? "" :
            dataCopy[std::distance(dataCopy.begin(), i)].substr(dataCopy[std::distance(dataCopy.begin(), i)].find_first_not_of(" \n\r\t\f\v"));
        std::string line = dataCopy[std::distance(dataCopy.begin(), i)];

        // separate line into words
        std::string splitToken;
        std::stringstream lineStream(line);
        std::vector<std::string> wordList;
        while (getline(lineStream, splitToken, ' ')) {
            wordList.push_back(splitToken);
        }

        // // dump lines to terminal
        // for (int i = 0; i < wordList.size(); i++) {
        //     std::cout << ">" << wordList[i] << "<" << std::endl;
        // }
        // std::cout << "line " << std::distance(dataCopy.begin(), i) + 1 << ": " << *i << "\n";
    }

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
        if (!lexer(fileName, &data)) return false;
        raw.close();
        return valid;
    } else {
        std::cout << "compilation interrupted\n";
        return false;
    }
    return false;
}