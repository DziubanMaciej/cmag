#pragma once

#include "cmag_core/core/cmag_project.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>

class CmagProject;

class CmagJsonWriter {
public:
    static void writeProject(const CmagProject &project, std::ostream &out);

private:
    static nlohmann::json createGlobalsNode(const CmagGlobals &globals);
    static nlohmann::json createGlobalValueListDirs(const CmagGlobals &globals);
    static nlohmann::json createListDirsNode(const CmagListDir &listDir, const CmagGlobals &globals);

    static nlohmann::json createTargetsNode(const std::vector<CmagTarget> &targets);
    static nlohmann::json createTargetNode(const CmagTarget &target);
    static nlohmann::json createTargetGraphicalNode(const CmagTargetGraphicalData &graphicalData);
    static nlohmann::json createConfigsNode(const std::vector<CmagTargetConfig> &configs);
    static nlohmann::json createConfigNode(const CmagTargetConfig &config);
};
