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
    static nlohmann::json createConfigNode(const std::vector<CmagTarget> &targets, std::string_view config);

    static std::optional<nlohmann::json> createTargetNode(const CmagTarget &target, std::string_view config);
    static nlohmann::json createPropertiesNode(const std::vector<CmagTarget::Property> &properties);
};
