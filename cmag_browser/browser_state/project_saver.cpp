#include "project_saver.h"

#include "cmag_core/parse/cmag_json_writer.h"
#include "cmag_core/utils/error.h"

#include <fstream>

ProjectSaver::ProjectSaver(CmagProject &project, size_t autoSaveIntervalMilliseconds)
    : project(project),
      lastSaveTime(Clock::now()),
      autoSaveInterval(std::chrono::milliseconds(autoSaveIntervalMilliseconds)) {}

void ProjectSaver::tryAutoSave() {
    // TODO
    // if (!project.getGlobals().enableAutoSave) {
    //    return;
    //}
    if (!isDirty()) {
        return;
    }

    const auto timeSinceLastSave = Clock::now() - lastSaveTime;
    if (timeSinceLastSave < autoSaveInterval) {
        return;
    }

    save();
}

void ProjectSaver::makeDirty(ProjectDirtyFlag flag) {
    dirtyState = dirtyState | flag;
}

void ProjectSaver::save() {
    lastSaveTime = Clock::now();

    // Open file
    const char *filePath = "/home/maciej/a.cmag-project.save";
    std::ofstream file{filePath};
    if (!file) {
        LOG_WARNING("Failed to open file ", filePath, " for saving the project.");
        return;
    }

    // Write project
    CmagJsonWriter::writeProject(project, file);
    if (!file) {
        LOG_WARNING("Failed saving the project.");
        return;
    }

    // Move to actual destination
    // TODO

    // Clear dirty flag
    dirtyState = ProjectDirtyFlag::None;
}

bool ProjectSaver::isDirty() const {
    return hasProjectDirtyFlagBit(dirtyMaskWindowTitleStar, dirtyState);
}

bool ProjectSaver::shouldShowDirtyNotification() const {
    return hasProjectDirtyFlagBit(dirtyMaskWindowTitleStar, dirtyState);
}
