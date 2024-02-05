#pragma once

#include "cmag_core/utils/enum_utils.h"
#include "cmag_core/utils/filesystem.h"

#include <chrono>

class CmagProject;

enum class ProjectDirtyFlag {
    None = 0,
    NodePosition = 1,
    CameraPosition = 2,
    SelectedConfig = 4,
    SelectedTab = 8,
    SelectedDependencies = 16,
    SelectedTarget = 32,
};

BITFIELD_ENUM(ProjectDirtyFlag)

class ProjectSaver {
public:
    ProjectSaver(CmagProject &project, const fs::path &outputPath, size_t autoSaveIntervalMilliseconds);

    void tryAutoSave(size_t frameIndex);
    void trySaveFromKeyboardShortcut();
    void makeDirty(ProjectDirtyFlag flag);
    void save();

    bool isDirty() const;
    bool shouldShowDirtyNotification() const;

private:
    using Clock = std::chrono::steady_clock;
    CmagProject &project;
    const fs::path outputPath;
    Clock::time_point lastSaveTime;
    const Clock::duration autoSaveInterval;
    ProjectDirtyFlag dirtyState = ProjectDirtyFlag::None;

    constexpr static inline ProjectDirtyFlag dirtyMaskAutoSave =
        ProjectDirtyFlag::NodePosition |
        ProjectDirtyFlag::CameraPosition |
        ProjectDirtyFlag::SelectedConfig |
        ProjectDirtyFlag::SelectedTab |
        ProjectDirtyFlag::SelectedDependencies |
        ProjectDirtyFlag::SelectedTarget;
    constexpr static inline ProjectDirtyFlag dirtyMaskWindowTitleStar =
        ProjectDirtyFlag::NodePosition |
        ProjectDirtyFlag::CameraPosition |
        ProjectDirtyFlag::SelectedConfig |
        ProjectDirtyFlag::SelectedDependencies;
};