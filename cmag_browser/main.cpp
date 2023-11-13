#include "main_window.h"

#include "cmag_lib/core/cmag_project.h"
#include "cmag_lib/parse/cmag_json_parser.h"

#include <QApplication>
#include <fstream>
#include <iostream>
#include <sstream>

std::optional<std::string> readFile(const fs::path &path) {
    std::ifstream stream(path);
    if (!stream) {
        return {};
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "ERROR: Specify cmag project file.\n";
        return 1;
    }
    const char *cmagProjectJsonPath = argv[1];
    const auto cmagProjectJson = readFile(cmagProjectJsonPath);
    if (!cmagProjectJson.has_value()) {
        std::cerr << "ERROR: could not read " << cmagProjectJsonPath << "\n";
        return 1;
    }

    CmagProject cmagProject = {};
    if (CmagJsonParser::parseProject(cmagProjectJson.value(), cmagProject) != ParseResult::Success) {
        std::cerr << "ERROR: could not parse cmag project file.\n";
        return 1;
    }

    int argcOne = 1;
    QApplication application(argcOne, argv);
    MainWindow w{nullptr, cmagProject};
    w.show();
    return QApplication::exec();
}
