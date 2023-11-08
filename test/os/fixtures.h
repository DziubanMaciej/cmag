#pragma once

#include "test/os/utils/file_operations.h"
#include "test/os/utils/test_workspace.h"

using CmagOsTest = ::testing::Test;

struct RaiiStdoutCapture {
    RaiiStdoutCapture() {
        testing::internal::CaptureStdout();
    }

    ~RaiiStdoutCapture() {
        testing::internal::GetCapturedStdout();
    }
};
