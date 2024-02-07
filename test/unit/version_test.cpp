#include "cmag_core/core/version.h"

#include <gtest/gtest.h>

TEST(VersionTest, whenCompileTimeConstructorIsUsedThenConstructCorrectVersion) {
    {
        CmagVersion version = CmagVersion::fromComponents<0, 0, 0>();
        EXPECT_EQ(0, version.comp0);
        EXPECT_EQ(0, version.comp1);
        EXPECT_EQ(0, version.comp2);
    }
    {
        CmagVersion version = CmagVersion::fromComponents<4, 13, 2>();
        EXPECT_EQ(4, version.comp0);
        EXPECT_EQ(13, version.comp1);
        EXPECT_EQ(2, version.comp2);
    }
    {
        CmagVersion version = CmagVersion::fromComponents<255, 65535, 255>();
        EXPECT_EQ(255, version.comp0);
        EXPECT_EQ(65535, version.comp1);
        EXPECT_EQ(255, version.comp2);
    }
}

TEST(VersionTest, givenCorrectArgumentsWhenRunTimeConstructorIsUsedThenConstructCorrectVersion) {
    {
        std::optional<CmagVersion> version = CmagVersion::fromComponents(0, 0, 0);
        ASSERT_TRUE(version.has_value());
        EXPECT_EQ(0, version.value().comp0);
        EXPECT_EQ(0, version.value().comp1);
        EXPECT_EQ(0, version.value().comp2);
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromComponents(4, 13, 2);
        ASSERT_TRUE(version.has_value());
        EXPECT_EQ(4, version.value().comp0);
        EXPECT_EQ(13, version.value().comp1);
        EXPECT_EQ(2, version.value().comp2);
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromComponents(255, 65535, 255);
        ASSERT_TRUE(version.has_value());
        EXPECT_EQ(255, version.value().comp0);
        EXPECT_EQ(65535, version.value().comp1);
        EXPECT_EQ(255, version.value().comp2);
    }
}

TEST(VersionTest, givenIncorrectArgumentsWhenRunTimeConstructorIsUsedThenConstructCorrectVersion) {
    {
        std::optional<CmagVersion> version = CmagVersion::fromComponents(256, 0, 0);
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromComponents(0, 65536, 0);
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromComponents(0, 0, 256);
        EXPECT_FALSE(version.has_value());
    }
}

TEST(VersionTest, givenCorrectArgumentsWhenFromStringConstructorIsUsedThenConstructCorrectVersion) {
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("0.0.0");
        ASSERT_TRUE(version.has_value());
        EXPECT_EQ(0, version.value().comp0);
        EXPECT_EQ(0, version.value().comp1);
        EXPECT_EQ(0, version.value().comp2);
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("4.13.2");
        ASSERT_TRUE(version.has_value());
        EXPECT_EQ(4, version.value().comp0);
        EXPECT_EQ(13, version.value().comp1);
        EXPECT_EQ(2, version.value().comp2);
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("255.65535.255");
        ASSERT_TRUE(version.has_value());
        EXPECT_EQ(255, version.value().comp0);
        EXPECT_EQ(65535, version.value().comp1);
        EXPECT_EQ(255, version.value().comp2);
    }
}

TEST(VersionTest, givenIncorrectArgumentsWhenFromStringTimeConstructorIsUsedThenConstructCorrectVersion) {
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("256.0.0");
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("0.65536.0");
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("0.0.256");
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("0.0");
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("hello");
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("1.1.1.");
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("1.1.1hello");
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("1.1.1 ");
        EXPECT_FALSE(version.has_value());
    }
    {
        std::optional<CmagVersion> version = CmagVersion::fromString("1.1,1");
        EXPECT_FALSE(version.has_value());
    }
}

TEST(VersionTest, givenVersionWhenCallingToStringThenProduceCorrectString) {
    {
        CmagVersion version = CmagVersion::fromComponents<0, 0, 0>();
        EXPECT_STREQ("0.0.0", version.toString().c_str());
    }
    {
        CmagVersion version = CmagVersion::fromComponents<4, 13, 2>();
        EXPECT_STREQ("4.13.2", version.toString().c_str());
    }
    {
        CmagVersion version = CmagVersion::fromComponents<255, 65535, 255>();
        EXPECT_STREQ("255.65535.255", version.toString().c_str());
    }
}

TEST(VersionTest, givenTwoVersionsWhenCheckingCompatibilityThenItWorksCorrectly) {
    EXPECT_TRUE((CmagVersion::fromComponents<100, 100, 100>().isProjectCompatible(CmagVersion::fromComponents<100, 100, 100>())));

    EXPECT_FALSE((CmagVersion::fromComponents<99, 100, 100>().isProjectCompatible(CmagVersion::fromComponents<100, 100, 100>())));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 99, 100>().isProjectCompatible(CmagVersion::fromComponents<100, 100, 100>())));
    EXPECT_TRUE((CmagVersion::fromComponents<100, 100, 99>().isProjectCompatible(CmagVersion::fromComponents<100, 100, 100>())));

    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>().isProjectCompatible(CmagVersion::fromComponents<99, 100, 100>())));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>().isProjectCompatible(CmagVersion::fromComponents<100, 99, 100>())));
    EXPECT_TRUE((CmagVersion::fromComponents<100, 100, 100>().isProjectCompatible(CmagVersion::fromComponents<100, 100, 99>())));
}

TEST(VersionTest, givenTwoVersionsWhenCallingEqualsOperatorThenItWorksCorrectly) {
    EXPECT_TRUE((CmagVersion::fromComponents<100, 100, 100>()) == (CmagVersion::fromComponents<100, 100, 100>()));

    EXPECT_FALSE((CmagVersion::fromComponents<99, 100, 100>()) == (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 99, 100>()) == (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 99>()) == (CmagVersion::fromComponents<100, 100, 100>()));

    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) == (CmagVersion::fromComponents<99, 100, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) == (CmagVersion::fromComponents<100, 99, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) == (CmagVersion::fromComponents<100, 100, 99>()));
}

TEST(VersionTest, givenTwoVersionsWhenCallingNotEqualsOperatorThenItWorksCorrectly) {
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) != (CmagVersion::fromComponents<100, 100, 100>()));

    EXPECT_TRUE((CmagVersion::fromComponents<99, 100, 100>()) != (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_TRUE((CmagVersion::fromComponents<100, 99, 100>()) != (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_TRUE((CmagVersion::fromComponents<100, 100, 99>()) != (CmagVersion::fromComponents<100, 100, 100>()));

    EXPECT_TRUE((CmagVersion::fromComponents<100, 100, 100>()) != (CmagVersion::fromComponents<99, 100, 100>()));
    EXPECT_TRUE((CmagVersion::fromComponents<100, 100, 100>()) != (CmagVersion::fromComponents<100, 99, 100>()));
    EXPECT_TRUE((CmagVersion::fromComponents<100, 100, 100>()) != (CmagVersion::fromComponents<100, 100, 99>()));
}

TEST(VersionTest, givenTwoVersionsWhenCallingLessThanOperatorThenItWorksCorrectly) {
    EXPECT_TRUE((CmagVersion::fromComponents<99, 100, 100>()) < (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) < (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) < (CmagVersion::fromComponents<99, 100, 100>()));

    EXPECT_TRUE((CmagVersion::fromComponents<100, 99, 100>()) < (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) < (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) < (CmagVersion::fromComponents<100, 99, 100>()));

    EXPECT_TRUE((CmagVersion::fromComponents<100, 100, 99>()) < (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) < (CmagVersion::fromComponents<100, 100, 100>()));
    EXPECT_FALSE((CmagVersion::fromComponents<100, 100, 100>()) < (CmagVersion::fromComponents<100, 100, 99>()));

    EXPECT_TRUE((CmagVersion::fromComponents<99, 100, 100>()) < (CmagVersion::fromComponents<100, 200, 200>()));
    EXPECT_TRUE((CmagVersion::fromComponents<100, 99, 100>()) < (CmagVersion::fromComponents<100, 100, 200>()));
}
