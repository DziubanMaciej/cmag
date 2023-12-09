#pragma once

#include "cmag_core/utils/filesystem.h"

#include <vector>

class BrowserArgumentParser {
public:
    BrowserArgumentParser(int argc, const char **argv);

    auto isValid() const { return valid; }
    const auto &getErrorMessage() const { return errorMessage; }
    const auto &getProjectFilePath() const { return projectFilePath; }
    auto getShowVersion() const { return showVersion; }

    void printHelp();

private:
    void setErrorMessage(const std::string &message);

    bool valid = true;
    std::string errorMessage = {};
    std::string projectFilePath = {};
    bool showVersion = false;
};
