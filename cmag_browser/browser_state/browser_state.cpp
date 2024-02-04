#include "cmag_browser/browser_state/browser_state.h"

BrowserState::BrowserState(const CmagBrowserTheme &theme, const fs::path &projectFilePath, CmagProject &project)
    : theme(theme),
      project(project),
      configSelector(theme, project, projectSaver),
      targetSelection(project),
      tabChange(*this),
      projectSaver(project, projectFilePath, 10000) {}
