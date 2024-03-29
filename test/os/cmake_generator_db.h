#pragma once

#include "cmag_core/core/cmake_generator.h"
#include "cmag_core/utils/subprocess.h"

#include <memory>

struct CmakeGeneratorDb {
    static void init(bool allGenerators) {
        inst = std::unique_ptr<CmakeGeneratorDb>(new CmakeGeneratorDb(allGenerators));
    }

    static const CmakeGeneratorDb &instance() {
        return *inst;
    }

    std::vector<CMakeGenerator> generators;

private:
    static bool isGeneratorAvailable(const char *name);

    explicit CmakeGeneratorDb(bool allGenerators);
    static inline std::unique_ptr<CmakeGeneratorDb> inst = {};
};
