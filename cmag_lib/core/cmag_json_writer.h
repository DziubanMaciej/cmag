#pragma once

#include "cmag_lib/core/cmag_project.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>

class CmagProject;

class CmagJsonWriter {
public:
    static void writeProject(const CmagProject &project, std::ostream &out);

private:
    static nlohmann::json createGlobalsNode(const CmagGlobals &globals);

    static nlohmann::json createTargetsNode(const std::vector<CmagTarget> &targets);
    static nlohmann::json createTargetNode(const CmagTarget &target);
    static nlohmann::json createConfigsNode(const std::vector<CmagTarget::Properties> &configs);
    static nlohmann::json createConfigNode(const std::vector<CmagTarget::Property> &config);
};
