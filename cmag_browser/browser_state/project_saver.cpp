#include "project_saver.h"

#include "cmag_core/parse/cmag_json_writer.h"
#include "cmag_core/utils/error.h"

#include <fstream>

ProjectSaver::ProjectSaver(CmagProject &project, const fs::path &outputPath, size_t autoSaveIntervalMilliseconds)
    : project(project),
      outputPath(outputPath),
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
    fs::path tmpOutputPath = outputPath;
    tmpOutputPath.replace_extension(outputPath.extension().string() + ".save");
    std::ofstream file{tmpOutputPath};
    if (!file) {
        LOG_WARNING("Failed to open file ", tmpOutputPath.string(), " for saving the project.");
        return;
    }

    // Write project
    CmagJsonWriter::writeProject(project, file);
    if (!file) {
        LOG_WARNING("Failed saving the project.");
        return;
    }

    // Move to actual destination
    std::error_code renameError{};
    fs::rename(tmpOutputPath, outputPath, renameError);
    if (renameError) {
        LOG_WARNING("Failed saving the project to original path. Project saved to ", tmpOutputPath, " - a backup path.");
    }

    // Clear dirty flag
    dirtyState = ProjectDirtyFlag::None;
}

bool ProjectSaver::isDirty() const {
    return hasProjectDirtyFlagBit(dirtyMaskWindowTitleStar, dirtyState);
}

bool ProjectSaver::shouldShowDirtyNotification() const {
    return hasProjectDirtyFlagBit(dirtyMaskWindowTitleStar, dirtyState);
}
