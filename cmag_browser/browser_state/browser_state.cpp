#include "cmag_browser/browser_state/browser_state.h"

BrowserState::BrowserState(const CmagBrowserTheme &theme, CmagProject &project)
    : theme(theme),
      project(project),
      configSelector(theme, project, projectSaver),
      tabChange(*this),
      projectSaver(project, 10000) {}
