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

std::string scanEscapeCharacters(ErrorHandler errors, int lineNumber, std::string line, std::string inputString, int index) {
    // escapes: n, \, ", ', {, },
    // for (int index = 0; index < inputString.length(); index++) {
        if (inputString[index] == '\\') {
            if (index >= inputString.length()) {
                errors.addError(lineNumber, "escape sequence incompleted", line);
            } else {
                if (inputString[index + 1] == 'n') {;
                    // inputString[index + 1] = '\n';
                    // inputString[index] = ' '; substring
                    inputString =
                        inputString.substr(0, index) +
                        "\n" +
                        inputString.substr(index + 2, inputString.length() - (index + 2))
                    ;
                }
                else if (inputString[index + 1] == '\\') {
                    inputString =
                        inputString.substr(0, index) +
                        "\\" +
                        inputString.substr(index + 2, inputString.length() - (index + 2))
                    ;
                }
                else if (inputString[index + 1] == '\"') {
                    inputString =
                        inputString.substr(0, index) +
                        "\"" +
                        inputString.substr(index + 2, inputString.length() - (index + 2))
                    ;
                }
                else if (inputString[index + 1] == '\'') {
                    inputString =
                        inputString.substr(0, index) +
                        "\'" +
                        inputString.substr(index + 2, inputString.length() - (index + 2))
                    ;
                }
                else if (inputString[index + 1] == 't') {
                    inputString =
                        inputString.substr(0, index) +
                        "\t" +
                        inputString.substr(index + 2, inputString.length() - (index + 2))
                    ;
                }
            }
        }
    // }
    return inputString;
}