
#include "test/os/cmake_generator_db.h"

#include <gtest/gtest.h>

int main(int argc, char *argv[]) {
    CmakeGeneratorDb::init(false); // initialize the db

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
