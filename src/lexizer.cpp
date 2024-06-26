#include <iostream>
#include <cstdio>
#include <vector>
#include <array>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <map>
#include "common.h"
#include "ErrorHandler.h"
#include "lexizer.h"


bool lexizer(std::string fileName, std::vector<std::string> *data) {
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
            // std::cout << architecture << " -> " << wordList[v] << std::endl;
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
                    if (wordList[j] == "//") break;
                    plaintextString += wordList[j] + " ";
                }
                v = wordList.size();
                plaintextString = plaintextString.substr(0, plaintextString.length() - 1);
            }

            // variables
            else if ((architecture == "view" || architecture == "meta") && (wordList[v] == "global" || wordList[v] == "signal" || wordList[v] == "var" || wordList[v] == "const")) {
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
            else if (architecture == "meta" && (wordList[v] == "page_title:" || wordList[v] == "search_tags:" || wordList[v] == "page_background:")) {
                if (changingPageSetting == "") {
                    changingPageSetting = wordList[v].substr(0, wordList[v].length() - 1);
                    plaintext = true;
                }
            }

            else if (architecture == "components" && wordList[v] == "create") {
                if (v != 0) errors.addError(lineNumber, "create is a reserved keyword", line);
                else {
                    v = wordList.size();
                    std::string varName;
                    std::vector<std::string> varParameters;
                    varName = wordList[1];
                    if (wordList.size() > 2) {
                        if (wordList[2] != "=") errors.addError(lineNumber, "variable name cannot be greater than one word", line);
                        else {
                            for (int j = 3; j < wordList.size(); j++) {
                                varParameters.push_back(wordList[j]);
                            }
                        }
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

        // semi-tokenizing settings
        if (changingPageSetting != "" && changingPageSetting != "search_tags") {
            if (plaintextString[0] == '[' && plaintextString.substr(plaintextString.length() - 1, 1) == "]") {
                plaintextString = plaintextString.substr(1, plaintextString.length() - 2);
                // std::cout << changingPageSetting << "\n" << plaintextString << "\n";
                int spacing = 0;
                bool inString = false;
                bool inKV = false;
                // if a setting is configured with no , at the end
                int startExit = 0;
                for (int v = 0; v < plaintextString.length(); v++) {
                    if (plaintextString[v] == ' ') {
                        spacing++; continue;
                    }

                    // string openers
                    if (plaintextString[v] == '\"' | plaintextString[v] == '\'') {
                        if (v - 1 > 0) {
                            if (plaintextString[v - 1] != '\\') inString = !inString;
                        } else inString = !inString;
                    }
                    // key value separator
                    // std::cout << plaintextString[v] << " == " << inString << "\n";
                    if (inString) plaintextString = scanEscapeCharacters(errors, lineNumber, line, plaintextString, v);
                    int start = 0;
                    if (plaintextString[v] == ':' && !inString) {
                        int parsedPairs = 0;
                        bool inString2 = false;
                        for (int j = 0; j < v; j++) {
                            if (plaintextString[j] == '\"' | plaintextString[j] == '\'') {
                                if (j - 1 > 0) {
                                    if (plaintextString[j - 1] != '\\') inString2 = !inString2;
                                } else inString2 = !inString2;
                            }
                            if (!inString2) {
                                if (plaintextString[j] == ':' || plaintextString[j] == ',') {
                                    parsedPairs++;
                                    if (j + 1 > plaintextString.length()) errors.addError(lineNumber, "setting left open", line);
                                    else {
                                        start = j + 1;
                                        startExit = j + 1;
                                        while (plaintextString[start] == ' ' || plaintextString[start] == ',') {
                                            start++;
                                            startExit++;
                                        }
                                    }
                                }
                            }
                        }
                        // std::cout << start << "\n";
                        if (parsedPairs % 2 != 0) errors.addError(lineNumber, "setting left open", line);
                        else inKV = true;
                        // std::cout << changingPageSetting << "\n" << inKV << "\n";
                    }
                    // if (inKV) std::cout << plaintextString[v] << "\n";
                    else if (plaintextString[v] == ',' && !inKV && !inString) errors.addError(lineNumber, "setting unopened", line);
                    else if (plaintextString[v] == ',' && inKV && !inString) {
                        std::string key = "";
                        std::string value = "";
                        bool settingKey = true;
                        bool inString2 = false;
                        for (int j = start; j < plaintextString.length(); j++) {
                            if (plaintextString[j] == '\"' | plaintextString[j] == '\'') {
                                if (j - 1 > 0) {
                                    if (plaintextString[j - 1] != '\\') inString2 = !inString2;
                                } else inString2 = !inString2;
                            }
                            // std::cout << ">" << plaintextString[j] << "<" << " // inString2 == " << inString2 << "\n";
                            if (!inString2) {
                                if (plaintextString[j] == ':') {
                                    settingKey = false;
                                    j++;
                                }
                                if (plaintextString[j] == ',') {
                                    inKV = false;
                                    j = plaintextString.length();
                                }
                            }
                            if (settingKey) key += plaintextString[j];
                            else value += plaintextString[j];
                        }
                        // trim spaces from start and end of value
                        int index = 0;
                        while (value[index] == ' ') {
                            index++;
                            value = value.substr(index);
                        }
                        value = value.substr(0, value.length() - 1);
                        pageSettingsInsert[key] = value;
                        // std::cout << key << ":" << value << "\n";
                    }
                    // else std::cout << "idk >" << plaintextString[v] << "<\n";
                    // std::cout << "kv: ";
                    // for (std::map<std::string, std::string>::iterator it = kv.begin(); it != kv.end(); it++) {
                    //     std::cout << it->first << ":" << it->second << " || ";
                    // } std::cout << std::endl;
                }
                // finalize key values
                if (inKV) {
                    std::string key = "";
                    std::string value = "";
                    bool settingKey = true;
                    bool inString2 = false;
                    for (int j = startExit; j < plaintextString.length(); j++) {
                        if (plaintextString[j] == '\"' | plaintextString[j] == '\'') {
                            if (j - 1 > 0) {
                                if (plaintextString[j - 1] != '\\') inString2 = !inString2;
                            } else inString2 = !inString2;
                        }
                        // std::cout << ">" << plaintextString[j] << "<" << " // inString2 == " << inString2 << "\n";
                        if (!inString2) {
                            if (plaintextString[j] == ':') {
                                settingKey = false;
                                j++;
                            }
                            if (plaintextString[j] == ',') {
                                inKV = false;
                                j = plaintextString.length();
                            }
                        }
                        if (settingKey) key += plaintextString[j];
                        else value += plaintextString[j];
                    }
                    // trim spaces from start and end of value
                    int index = 0;
                    while (value[index] == ' ') {
                        index++;
                        value = value.substr(index);
                    }
                    index = value.length() - 1;
                    while (value[index] == ' ') {
                        index--;
                        value = value.substr(0, index);
                    }
                    pageSettingsInsert[key] = value;
                }

                if (inString) errors.addError(lineNumber, "string left opened", line);
                // std::cout << changingPageSetting << ", inString: " << inString << "\n>" << plaintextString << "<\n\n";
            }
        }

        // because search_tags has no keys
        else if (changingPageSetting == "search_tags") {
            if (plaintextString[0] == '[' && plaintextString.substr(plaintextString.length() - 1, 1) == "]") {
                plaintextString = plaintextString.substr(1, plaintextString.length() - 2);
                std::cout << "search tags: " << plaintextString << "\n";
                bool inString2 = false;
                int key = 0;
                int valueStart = 0;
                for (int v = 0; v < plaintextString.length(); v++) {
                    if (plaintextString[v] == '\"' | plaintextString[v] == '\'') {
                        if (v - 1 > 0) {
                            if (plaintextString[v - 1] != '\\') inString2 = !inString2;
                        } else inString2 = !inString2;
                    }
                    if (inString2) plaintextString = scanEscapeCharacters(errors, lineNumber, line, plaintextString, v);
                    else {
                        if (plaintextString[v] != ',' && plaintextString[v] != ' ' && plaintextString[v] != '\"' && plaintextString[v] != '\'') errors.addError(lineNumber, "invalid token", line);
                        else if (plaintextString[v] == ',') {
                            std::string value = "";
                            for (int j = valueStart; j < v; j++) {
                                value += plaintextString[j];
                            }
                            if (v + 1 > plaintextString.length() - 1) errors.addError(lineNumber, "setting incomplete / overflow", line);
                            else valueStart = v + 1;
                            pageSettingsInsert[std::to_string(key)] = value;
                            key++;
                        }
                    }
                }
                std::string value = "";
                for (int j = valueStart; j < plaintextString.length() - 1; j++) {
                    value += plaintextString[j];
                }
                pageSettingsInsert[std::to_string(key)] = value;
            }
        }

        // page_title additions
        if (changingPageSetting == "page_title") {
            // default for "text"
            if (pageSettingsInsert.find("text") == pageSettingsInsert.end()) {
                std::string version = std::to_string(VERSION);
                version.erase(version.find_last_not_of('0') + 1, std::string::npos);
                version.erase(version.find_last_not_of('.') + 1, std::string::npos);
                pageSettingsInsert["text"] = "Alchemy " + version;
            }
        }

        // clean up page settings insert here, removing quotes and spaces and such

        // adding page settings <-- should work on all page settings if they're set up
        if (changingPageSetting != "") pageSettings[changingPageSetting] = pageSettingsInsert;

        // // dump words on line to terminal
        // for (int i = 0; i < wordList.size(); i++) {
        //     std::cout << ">" << wordList[i] << "<" << std::endl;
        // }
        // // dump lines to terminal
        // std::cout << "line " << lineNumber << ": " << *i << "\n";
    }

    // for (std::map<std::string, std::string>::iterator it = pageSettings.begin(); it != pageSettings.end(); it++) {
    //     std::cout << it->first << ":" << it->second << " || ";
    // } std::cout << std::endl;

    // for (auto const& x : pageSettings) {
    //     std::cout << x.first << "\n";
    //     for (auto it = x.second.begin(); it != x.second.end(); it++) {
    //         std::cout << it->first << ":" << it->second << "\n";
    //     } std::cout << "\n";
    // }

    return errors.catcher();
}