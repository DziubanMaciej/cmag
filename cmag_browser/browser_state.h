#pragma once

#include "cmag_browser/config_selector.h"

class BrowserState {
public:
    BrowserState(const CmagBrowserTheme &theme, CmagProject &project);

    const auto &getTheme() { return theme; }
    auto &getProject() { return project; }
    auto &getConfigSelector() { return configSelector; }

private:
    const CmagBrowserTheme &theme;
    CmagProject &project;
    ConfigSelector configSelector;
};
