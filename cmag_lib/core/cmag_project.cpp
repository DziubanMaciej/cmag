#include "cmag_project.h"

void CmagProject::addTarget(CmagTarget &&target) {
    targets.push_back(std::move(target));
}
