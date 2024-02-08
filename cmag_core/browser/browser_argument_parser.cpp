#include "browser_argument_parser.h"

#include "cmag_core/utils/error.h"

#include <generated/cmag_browser_cli.h>

BrowserArgumentParser::BrowserArgumentParser(int argc, const char **argv) {
    for (int argIndex = 1; argIndex < argc; argIndex++) {
        const std::string arg = argv[argIndex];

        // Ignore empty args
        if (arg.empty()) {
            continue;
        }

        // Parse arguments known to us
        bool validArg = false;
        if (arg == "-v") {
            showVersion = true;
            validArg = true;
        } else if (arg == "-d") {
            showDebugWidgets = true;
            validArg = true;
        } else if (arg[0] == '-') {
            setErrorMessage(LOG_TO_STRING("Unknown option: ", arg));
        } else {
            if (!projectFilePath.empty()) {
                setErrorMessage(LOG_TO_STRING("Too many files specified: ", projectFilePath, ", ", arg));
            } else {
                projectFilePath = arg;
                validArg = true;
            }
        }

        // Check validity of current arg
        if (!validArg) {
            valid = false;
        }
    }

    // Verify arguments that were set
    if (projectFilePath.empty()) {
        setErrorMessage("Specify cmag project file.");
    }
    FATAL_ERROR_IF(!valid && errorMessage.empty(), "Error message must be set, when something is wrong");
}

void BrowserArgumentParser::printHelp() {
    printf("%s", cmagBrowserCli);
}
void BrowserArgumentParser::setErrorMessage(const std::string &message) {
    valid = false;
    errorMessage = message;
}
