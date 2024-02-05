#include "project_saver.h"

#include "cmag_core/parse/cmag_json_writer.h"
#include "cmag_core/utils/error.h"

#include <fstream>
#include <imgui/imgui.h>

ProjectSaver::ProjectSaver(CmagProject &project, const fs::path &outputPath, size_t autoSaveIntervalMilliseconds)
    : project(project),
      outputPath(outputPath),
      lastSaveTime(Clock::now()),
      autoSaveInterval(std::chrono::milliseconds(autoSaveIntervalMilliseconds)) {}

void ProjectSaver::tryAutoSave(size_t frameIndex) {
    if (frameIndex < 3) {
        // During the first few frames it is possible to set some unneeded dirty flags, because ImGui may have some delays
        // and cause ping-ponging between states. For example this happens when forcing selected tab in tab bar. The request
        // is honored only in second frame. With this workaround we hide this and avoid unneeded calls to save().
        dirtyState = ProjectDirtyFlag::None;
        return;
    }

    if (!project.getGlobals().browser.autoSaveEnabled) {
        return;
    }
    if (!isDirty()) {
        return;
    }

    const auto timeSinceLastSave = Clock::now() - lastSaveTime;
    if (timeSinceLastSave < autoSaveInterval) {
        return;
    }

    save();
}

void ProjectSaver::trySaveFromKeyboardShortcut() {
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)) {
        save();
    }
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
    return hasProjectDirtyFlagBit(dirtyMaskAutoSave, dirtyState);
}

bool ProjectSaver::shouldShowDirtyNotification() const {
    return hasProjectDirtyFlagBit(dirtyMaskWindowTitleStar, dirtyState);
}
