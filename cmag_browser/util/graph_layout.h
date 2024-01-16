#pragma once

#include "cmag_core/core/cmag_project.h"

#include <vector>

void calculateLayout(const std::vector<CmagTarget *> &targets,
                     std::string_view configName,
                     size_t nodeWidth,
                     size_t nodeHeight);
