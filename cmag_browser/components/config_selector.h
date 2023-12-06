#pragma once

#include <memory>
#include <string_view>

class CmagProject;
class CmagBrowserTheme;

class ConfigSelector {
public:
    ConfigSelector(const CmagBrowserTheme &theme, CmagProject &project);

    void render(float width, bool skipTooltip = false);
    void renderTooltip();
    std::string_view getCurrentConfig();

private:
    const CmagBrowserTheme &theme;
    CmagProject &project;
    int currentSelection;
    int selectionsCount;
    std::unique_ptr<const char *[]> configs;
};
