#pragma once

#include <string>
#include <string_view>
#include <vector>

enum class XdotParseResult {
    Success,
    Malformed,
    MissingField,
    Eof,
};

struct XdotData {
    struct Target {
        std::string name;
        float x;
        float y;
    };
    std::vector<Target> targets;
};

class XdotParser {
public:
    static XdotParseResult parse(std::string_view xdot, XdotData &outData);

private:
    static XdotParseResult skipToString(std::string_view &xdot, std::string_view expectedString);
    static XdotParseResult expectNumber(std::string_view &xdot);
    static XdotParseResult readNextNonWhitespace(std::string_view &xdot, char &outChar);
    static XdotParseResult readStringAlphabetic(std::string_view &xdot, std::string &outString);
    static XdotParseResult readStringFloatingPoint(std::string_view &xdot, float &outFloat);
};
