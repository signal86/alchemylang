#include <iostream>
#include <cstdio>
#include <vector>
#include <array>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <map>

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
                    "%s on %s:%d ->\n\t\t\"%s\"\n\t-> %s\n",
                    startingText.c_str(), fileName.c_str(), error.lineNumber, error.line.c_str(), error.error.c_str()
                );
            }

            return false;
        }
};

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

bool lexer(std::string fileName, std::vector<std::string> *data) {
    std::vector<std::string> dataCopy = *data;
    ErrorHandler errors("lexizer/syntax error", fileName);

    std::array<std::string, 11> reservedKeywords = {
        "global",
        "var",
        "const",
        "signal",
        "meta",
        "view",
        "create",
        "redirect",
        "on",
        "end",
        "if"
    };
    // file-level variables
    std::vector<Variable> variableScope; // scope
    std::map<std::string, std::map<std::string, std::string>> pageSettings; // settings of the page: [settingName: [setting: settingValue]]
    bool commenting = false;
    int blockNum = 0;
    std::string architecture = "none";
    for (auto i = dataCopy.begin(); i != dataCopy.end(); i++) {
        int lineNumber = std::distance(dataCopy.begin(), i) + 1;

        // beginning whitespace trim
        std::string line = dataCopy[std::distance(dataCopy.begin(), i)];
        line =
            (line.find_first_not_of(" \n\r\t\f\v") == std::string::npos) ? "" :
            line.substr(line.find_first_not_of(" \n\r\t\f\v"));

        // separate line into words
        std::string splitToken;
        std::stringstream lineStream(line);
        std::vector<std::string> wordList;
        while (getline(lineStream, splitToken, ' ')) {
            wordList.push_back(splitToken);
        }

        // line-level variables
        bool architectureInline = false; // for {
        bool settingVariable = false;
        bool plaintext = false;
        std::string plaintextString = "";
        Variable variableWork;
        std::map<std::string, std::string> pageSettingsInsert;
        std::string changingPageSetting = "";
        // loop all words in the line and search through word values
        // TODO: plaintext should overwrite existing string
        for (int v = 0; v < wordList.size(); v++) {
            // comments
            if (wordList[v] == "//") {
                v = wordList.size();
            }

            else if (wordList[v].rfind("/*", 0) == 0) {
                commenting = true;
            }

            else if (wordList[v].rfind("*/") != std::string::npos) {
                if (commenting) commenting = false;
                else errors.addError(lineNumber, "comment closed which was never opened", line);
            }

            else if (commenting) continue;

            // plaintext handler 1
            else if (plaintext) {
                for (int j = v; j < wordList.size(); j++) {
                    if (j != wordList.size() - 2) plaintextString += wordList[j] + " ";
                    else plaintextString += wordList[j];
                }
                v = wordList.size();
            }

            // variables
            else if ((architecture == "global" || architecture == "meta") && (wordList[v] == "global" || wordList[v] == "signal" || wordList[v] == "var" || wordList[v] == "const")) {
                if (v != 0 && settingVariable == false) {
                    errors.addError(lineNumber, "reserved keyword \"" + wordList[v] + "\" used outside of variable declaration", line);
                } else if (settingVariable == true || v == 0) {
                    settingVariable = true;
                    variableWork.scope = architecture;
                    if (wordList[v] == "global") {
                        variableWork.global = true;
                        variableWork.scope = "global";
                    }
                    else if (wordList[v] == "signal") variableWork.signal = true;
                    else if (wordList[v] == "const") {
                        if (variableWork.varUsed) errors.addError(lineNumber, "variable declared with both const and var", line);
                        else variableWork.constant = true;
                    }
                    else if (wordList[v] == "var") {
                        if (variableWork.constant) errors.addError(lineNumber, "variable declared with both const and var", line);
                        else variableWork.varUsed = true;
                    }
                }
            }

            // architecture blocks
            else if (wordList[v] == "meta" || wordList[v] == "view" || wordList[v] == "components" || wordList[v] == "global_modifiers") {
                if (architecture == "none") architecture = wordList[v];
                else errors.addError(lineNumber, "overlapping architecture blocks", line);
                int tempBlockNum = blockNum;
                bool found = false;
                // next character is a {
                if (v + 1 < wordList.size() && wordList[v + 1] == "{") {
                    architectureInline = true;
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
                // architecture block never opened
                else errors.addError(lineNumber, "architecture block not opened", line);
                if (!found) errors.addError(lineNumber, "architecture block left incomplete", line);
            }

            else if (wordList[v] == "{") {
                if (!architectureInline) errors.addError(lineNumber, "architecture block not found", line);
            }

            else if (wordList[v] == "}") {
                if (architecture == "none") errors.addError(lineNumber, "architecture block not found", line);
                else {
                    architecture = "none";
                    blockNum--;
                }
            }

            // page settings
            // right now, only allows for one-line variable setting. In the future, it may be most efficient and scalable to allow for implementation of data from other documents as a GLOBAL preprocessor variable, which can be called
            else if (architecture == "meta" && wordList[v] == "page_title:") {
                if (changingPageSetting != "") {
                    changingPageSetting = "page_title";
                    if (wordList.size() > v + 2 && wordList[v + 1] == "=") {
                        plaintext = true;
                    } else {
                        errors.addError(lineNumber, "page setting improperly defined", line);
                    }
                }
            }

            // preprocessors
            else if (wordList[v].rfind("#", 0) == 0) {
                v = wordList.size();
            }

            // un-reserved keyword handler
            else {
                // variable declaration
                // This probably needs a rewrite into moving toward the plaintext handler defined above
                if (settingVariable) {
                    if (variableWork.variableName == "") {
                        variableWork.variableName = wordList[v];
                    } else {
                        if (!variableWork.initialized && wordList[v] == "=") {
                            variableWork.initialized = true;
                        }
                        else if (variableWork.initialized) {
                            variableWork.value += wordList[v];
                        }
                        else {
                            errors.addError(lineNumber, "variable setting failed", line);
                        }
                    }
                }
                else {
                    errors.addError(lineNumber, "data not in scope", line);
                }
            }
        }

        // post-line processing
        // dealing with uninitialized global variables
        if (variableWork.constant && !variableWork.initialized) {
            errors.addError(lineNumber, "constant variables cannot be left uninitialized", line);
        }

        // adding variables to scope
        if (variableWork.variableName != "") variableScope.push_back(variableWork);

        // page_title mini-tokenization
        if (changingPageSetting == "page_title") {
            // if (no "text" key) {
                pageSettingsInsert["text"] = "Alchemy " + std::to_string(VERSION);
            // } else {
                // pageSettingsInsert["text"] = value;
            // }
        }

        // adding page settings <-- should work on all page settings if they're set up
        if (changingPageSetting != "") pageSettings[changingPageSetting] = pageSettingsInsert;

        // // dump words on line to terminal
        // for (int i = 0; i < wordList.size(); i++) {
        //     std::cout << ">" << wordList[i] << "<" << std::endl;
        // }
        // // dump lines to terminal
        // std::cout << "line " << lineNumber << ": " << *i << "\n";
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