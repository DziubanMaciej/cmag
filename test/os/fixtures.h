#pragma once

#include "cmag_lib/utils/filesystem.h"

#include <gtest/gtest.h>

#if _WIN32
#include <Windows.h>
#include <aclapi.h>
#endif

const static inline fs::path srcProjectsRoot = fs::path{SRC_PROJECTS_ROOT};
const static inline fs::path dstProjectsRoot = fs::path{DST_PROJECTS_ROOT};

struct CmagOsTest : ::testing::Test {
    struct TestWorkspace {
        bool valid = false;
        fs::path sourcePath;
        fs::path buildPath;
        fs::path graphvizPath;
    };

    void TearDown() override {
        for (auto &workspace : workspaces) {
            removeFile(workspace.sourcePath);
        }
    }

    TestWorkspace prepareWorkspace(std::string_view name) {
        const fs::path srcProjectDir = srcProjectsRoot / name;
        const fs::path dstProjectDir = dstProjectsRoot / name;

        // Cleanup src dir.
        EXPECT_TRUE(fs::is_regular_file(srcProjectDir / "CMakeLists.txt"));
        EXPECT_TRUE(fs::is_directory(srcProjectDir));
        removeFile(srcProjectDir / "build");

        // Cleanup dst dir.
        removeFile(dstProjectsRoot);
        createDir(dstProjectsRoot);
        createDir(dstProjectDir);

        // Copy src dir to dst dir.
        copyDir(srcProjectDir, dstProjectDir);
        createDir(dstProjectDir / "build");

        // Return workspace description
        TestWorkspace workspace = {};
        workspace.valid = !::testing::Test::HasFailure();
        workspace.sourcePath = dstProjectDir;
        workspace.buildPath = dstProjectDir / "build";
        workspace.graphvizPath = dstProjectDir / "build" / "a.dot";
        workspaces.push_back(workspace);
        return workspace;
    }

    static void createDir(const fs::path &dir) {
        std::error_code err{};
        fs::create_directory(dir, err);
        EXPECT_EQ(0, err.value());
    }

    static void copyDir(const fs::path &src, const fs::path &dst) {
        std::error_code err{};
        fs::copy(src, dst, fs::copy_options::recursive, err);
        EXPECT_EQ(0, err.value());
    }

    static void removeFile(const fs::path &dir) {
        if (fs::exists(dir)) {
            std::error_code err{};
            fs::remove_all(dir, err);
            EXPECT_EQ(0, err.value());
        }
        EXPECT_FALSE(fs::exists(dir));
    }

    static bool clearPermissions(const fs::path &file) {
#ifdef _WIN32
        // Prepare deny rule
        EXPLICIT_ACCESS denyAccess = {};
        denyAccess.grfAccessMode = DENY_ACCESS;
        denyAccess.grfAccessPermissions = GENERIC_ALL;
        denyAccess.grfInheritance = NO_INHERITANCE;
        denyAccess.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
        denyAccess.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        denyAccess.Trustee.ptstrName = "Everyone";

        // Create new ACL with the deny rule
        PACL newAcl = NULL;
        if (SetEntriesInAclA(1, &denyAccess, NULL, &newAcl) != ERROR_SUCCESS) {
            return false;
        }

        // Set the new ACL in the security descriptor.
        std::string filePath = file.string();
        const bool result = SetNamedSecurityInfoA(filePath.data(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, newAcl, NULL) == ERROR_SUCCESS;

        // Free created ACL
        LocalFree(newAcl);
        return result;
#else
        std::error_code err{};
        fs::permissions(file, fs::perms::none, err);
        return err.value() == 0;
#endif
    }

    static bool restorePermissions([[maybe_unused]] const fs::path &file) {
#ifdef _WIN32
        // Get the current ACL
        std::string filePath = file.string();
        PSECURITY_DESCRIPTOR securityDescriptor = NULL;
        if (GetNamedSecurityInfo(filePath.data(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, NULL, NULL, &securityDescriptor) != ERROR_SUCCESS) {
            return false;
        }
        PACL currentAcl = NULL;
        BOOL isAclPresent = FALSE;
        BOOL isAclDefaulted = FALSE;
        const bool aclQueried = GetSecurityDescriptorDacl(securityDescriptor, &isAclPresent, &currentAcl, &isAclDefaulted) != ERROR_SUCCESS;
        LocalFree(securityDescriptor);
        if (!aclQueried) {
            // Failed to query current ACL
            return false;
        } else if (!isAclPresent || currentAcl == nullptr || isAclDefaulted) {
            // ACL is either non-existant, empty or defaulted. All of these cases meanswe are not forbidden access, so no action is needed
            return true;
        }

        // Initialize a new ACL
        std::unique_ptr<char[]> newAclMemory = std::make_unique<char[]>(currentAcl->AclSize);
        PACL newAcl = reinterpret_cast<PACL>(newAclMemory.get());
        if (InitializeAcl(newAcl, currentAcl->AclSize, ACL_REVISION) == 0) {
            return false;
        }

        // Add entries from old ACL to new ACL, but skip 'deny' rules.
        for (DWORD i = 0; i < currentAcl->AceCount; i++) {
            ACCESS_ALLOWED_ACE *ace = NULL;
            if (GetAce(currentAcl, i, (LPVOID *)&ace) == 0) {
                EXPECT_TRUE(false);
                continue;
            }
            if (ace->Header.AceType == ACCESS_DENIED_ACE_TYPE) {
                continue;
            }

            EXPECT_NE(0, AddAce(newAcl, ACL_REVISION, 0, ace, ace->Header.AceSize));
        }

        // Set the new DACL in the security descriptor.
        const bool result = SetNamedSecurityInfoA(filePath.data(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, newAcl, NULL) == ERROR_SUCCESS;
        return result;
#else
        return true;
#endif
    }

private:
    std::vector<TestWorkspace> workspaces = {};
};

struct RaiiStdoutCapture {
    RaiiStdoutCapture() {
        testing::internal::CaptureStdout();
    }

    ~RaiiStdoutCapture() {
        testing::internal::GetCapturedStdout();
    }
};