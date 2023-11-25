#pragma once

// Local space is defined in range <-1, 1> for both x and y
constexpr float localSpaceHalfWidth = 1;
constexpr float localSpaceHalfHeight = 1;

// World space is defined in our arbitrary range.
constexpr float worldSpaceHalfWidth = 200;
constexpr float worldSpaceHalfHeight = 100;
constexpr float worldSpaceAspectRatio = worldSpaceHalfWidth / worldSpaceHalfHeight;

// Screen space depends on space in window that is available to us. Although, we will
// keep the aspect ratio the same as world space, so we don't stretch, squeeze or
// change the visible area.
constexpr float screenSpaceAspectRatio = worldSpaceAspectRatio;
