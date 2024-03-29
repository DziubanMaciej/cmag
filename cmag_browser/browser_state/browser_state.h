#pragma once

#include "cmag_browser/browser_state/config_selector.h"
#include "cmag_browser/browser_state/project_saver.h"
#include "cmag_browser/browser_state/tab_change.h"
#include "cmag_browser/browser_state/target_selection.h"

class BrowserState {
public:
    BrowserState(const CmagBrowserTheme &theme, const fs::path &projectFilePath, CmagProject &project);

    const auto &getTheme() { return theme; }
    auto &getProject() { return project; }
    auto &getConfigSelector() { return configSelector; }
    auto &getTargetSelection() { return targetSelection; }
    auto &getTabChange() { return tabChange; }
    auto &getProjectSaver() { return projectSaver; }

private:
    const CmagBrowserTheme &theme;
    CmagProject &project;
    ConfigSelector configSelector;
    TargetSelection targetSelection;
    TabChange tabChange;
    ProjectSaver projectSaver;
};
