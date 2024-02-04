#include "target_selection.h"

#include "cmag_core/core/cmag_project.h"

TargetSelection::TargetSelection(CmagProject &project) : project(project) {
    const std::string &selectedTargetName = project.getGlobals().browser.selectedTargetName;
    for (CmagTarget &target : project.getTargets()) {
        if (target.name == selectedTargetName) {
            selection = &target;
            break;
        }
    }
}

void TargetSelection::select(CmagTarget *target) {
    selection = target;
    project.getGlobals().browser.selectedTargetName = target->name;
}
