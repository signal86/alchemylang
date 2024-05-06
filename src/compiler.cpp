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

    int blockNum = 0;
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

        // loop all words in the line and search through word values
        for (int v = 0; v < wordList.size(); v++) {
            // architecture blocks
            if (wordList[v] == "meta" || wordList[v] == "view" || wordList[v] == "components" || wordList[v] == "global_modifiers") {
                if (architecture == "none") architecture = wordList[v];
                else errors.addError(std::distance(dataCopy.begin(), i) + 1, "overlapping architecture blocks", line);
                int tempBlockNum = blockNum;
                bool found = false;
                // next character is a {
                if (v + 1 < wordList.size() && wordList[v + 1] == "{") {
                    // reverse through all lines to find the opposite "}"
                    for (int j = dataCopy.size() - 1; j > 0; j--) {
                        // redo separation of lines into words
                        std::string line2 = dataCopy[j];
                        std::string splitToken2;
                        std::stringstream lineStream2(line2);
                        std::vector<std::string> wordList2;
                        while (getline(lineStream2, splitToken2, ' ')) {
                            wordList2.push_back(splitToken2);
                        }

                        // iterate through the words
                        for (int k = 0; k < wordList2.size(); k++) {
                            if (wordList2[k] == "}") {
                                if (tempBlockNum > 0) tempBlockNum--;
                                else {
                                    blockNum++;
                                    found = true;
                                }
                            }
                        }
                    }
                }
                // architecture block never opend
                else errors.addError(std::distance(dataCopy.begin(), i) + 1, "architecture block not opened", line);
                if (!found) errors.addError(std::distance(dataCopy.begin(), i) + 1, "architecture block left incomplete", line);
            }

            else if (wordList[v] == "}") {
                if (architecture == "none") errors.addError(std::distance(dataCopy.begin(), i) + 1, "architecture block not found", line);
                else {
                    architecture = "none";
                    blockNum--;
                }
            }

            else if (wordList[v] == "//") {
                v = wordList.size() - 1;
            }
        }

        // // dump words on line to terminal
        // for (int i = 0; i < wordList.size(); i++) {
        //     std::cout << ">" << wordList[i] << "<" << std::endl;
        // }
        // // dump lines to terminal
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