#pragma once

#include "cmag_browser/config_selector.h"
#include "cmag_browser/target_selection.h"

class BrowserState {
public:
    BrowserState(const CmagBrowserTheme &theme, CmagProject &project);

    const auto &getTheme() { return theme; }
    auto &getProject() { return project; }
    auto &getConfigSelector() { return configSelector; }
    auto &getTargetSelection() { return targetSelection; }

private:
    const CmagBrowserTheme &theme;
    CmagProject &project;
    ConfigSelector configSelector;
    TargetSelection targetSelection{};
};
