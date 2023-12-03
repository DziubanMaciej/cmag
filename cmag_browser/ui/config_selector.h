#pragma once

#include <memory>
#include <string_view>

class CmagProject;

class ConfigSelector {
public:
    explicit ConfigSelector(CmagProject &project);

    void render();
    std::string_view getCurrentConfig();

private:
    CmagProject &project;
    int currentSelection;
    int selectionsCount;
    std::unique_ptr<const char *[]> configs;
};
