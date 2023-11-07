#include "xdot_parser.h"

#define RETURN_ERROR(res)                      \
    do {                                       \
        const auto r = (res);                  \
        if ((r) != XdotParseResult::Success) { \
            return (r);                        \
        }                                      \
    } while (false)

XdotParseResult XdotParser::parse(std::string_view xdot, XdotData &outData) {
    RETURN_ERROR(skipToString(xdot, "clusterLegend {"));
    RETURN_ERROR(skipToString(xdot, "}"));

    while (true) {
        const XdotParseResult result = skipToString(xdot, "node");
        if (result == XdotParseResult::Eof) {
            break;
        }
        RETURN_ERROR(result);
        RETURN_ERROR(expectNumber(xdot));

        char character{};
        RETURN_ERROR(readNextNonWhitespace(xdot, character));

        switch (character) {
        case '-':
            // This is a connection - skip.
            RETURN_ERROR(skipToString(xdot, "]"));
            break;
        case '[': {
            // This is a node - process
            XdotData::Target target = {};

            RETURN_ERROR(skipToString(xdot, "label="));
            RETURN_ERROR(readOptionalChar(xdot, '"'));
            RETURN_ERROR(readTargetName(xdot, target.name));

            RETURN_ERROR(skipToString(xdot, "pos=\""));
            RETURN_ERROR(readStringFloatingPoint(xdot, target.x));

            char comma{};
            RETURN_ERROR(readNextNonWhitespace(xdot, comma));
            if (comma != ',') {
                return XdotParseResult::Malformed;
            }
            RETURN_ERROR(readStringFloatingPoint(xdot, target.y));

            outData.targets.push_back(std::move(target));
            break;
        }
        default:
            // Unexpected token - error
            return XdotParseResult::Malformed;
        }
    }

    return XdotParseResult::Success;
}

XdotParseResult XdotParser::skipToString(std::string_view &xdot, std::string_view expectedString) {
    const auto inputLen = xdot.length();
    const auto expectedStringLen = expectedString.length();

    size_t expectedStringIterator = 0;
    for (size_t i = 0; i < inputLen; i++) {
        if (xdot[i] == expectedString[expectedStringIterator]) {
            expectedStringIterator++;
            if (expectedStringIterator == expectedStringLen) {
                xdot.remove_prefix(i + 1);
                return XdotParseResult::Success;
            }
        } else {
            expectedStringIterator = 0;
        }
    }

    return XdotParseResult::Eof;
}

XdotParseResult XdotParser::expectNumber(std::string_view &xdot) {
    const auto inputLen = xdot.length();

    size_t digitsCount = 0;
    for (; digitsCount < inputLen; digitsCount++) {
        if (!::isdigit(xdot[digitsCount])) {
            break;
        }
    }

    if (digitsCount == 0) {
        return XdotParseResult::MissingField;
    }

    xdot.remove_prefix(digitsCount);
    return XdotParseResult::Success;
}

XdotParseResult XdotParser::readNextNonWhitespace(std::string_view &xdot, char &outChar) {
    const auto inputLen = xdot.length();

    for (size_t whitespaceCount = 0; whitespaceCount < inputLen; whitespaceCount++) {
        char nextChar = xdot[whitespaceCount];
        if (nextChar != ' ' && nextChar != '\t' && nextChar != '\n' && nextChar != '\r') {
            outChar = nextChar;
            xdot.remove_prefix(whitespaceCount + 1);
            return XdotParseResult::Success;
        }
    }

    return XdotParseResult::Eof;
}

XdotParseResult XdotParser::readOptionalChar(std::string_view &xdot, char optionalChar) {
    const auto inputLen = xdot.length();
    if (inputLen == 0) {
        return XdotParseResult::Eof;
    }

    if (xdot[0] == optionalChar) {
        xdot.remove_prefix(1);
    }
    return XdotParseResult::Success;
}

XdotParseResult XdotParser::readTargetName(std::string_view &xdot, std::string &outString) {
    const auto inputLen = xdot.length();

    size_t charCount = 0;
    for (; charCount < inputLen; charCount++) {
        char nextChar = xdot[charCount];
        // See specification of target name in CMake https://cmake.org/cmake/help/latest/policy/CMP0037.html
        if (!::isalpha(nextChar) && !::isdigit(nextChar) && nextChar != '_' && nextChar != '.' && nextChar != '+' && nextChar != '-') {
            break;
        }
    }

    if (charCount == 0) {
        return XdotParseResult::MissingField;
    }

    outString = xdot.substr(0, charCount);
    xdot.remove_prefix(charCount);
    return XdotParseResult::Success;
}

XdotParseResult XdotParser::readStringFloatingPoint(std::string_view &xdot, float &outFloat) {
    const auto inputLen = xdot.length();

    size_t charCount = 0;
    bool decimalPointSeen = false;
    for (; charCount < inputLen; charCount++) {
        char nextChar = xdot[charCount];
        if (!::isdigit(nextChar)) {
            if (nextChar != '.') {
                break;
            }

            if (decimalPointSeen) {
                return XdotParseResult::Malformed;
            }
            decimalPointSeen = true;
        }
    }

    if (charCount == 0) {
        return XdotParseResult::MissingField;
    }

    // TODO this is not very nice, but it will do for now.
    std::string floatString{xdot.substr(0, charCount)};
    sscanf(floatString.c_str(), "%f", &outFloat);
    xdot.remove_prefix(charCount);
    return XdotParseResult::Success;
}
