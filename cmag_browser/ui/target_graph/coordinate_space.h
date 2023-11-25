#pragma once

// World space is defined in range <-100, 100> for both x and y.
constexpr float worldSpaceHalfWidth = 200;
constexpr float worldSpaceHalfHeight = 100;
constexpr float worldSpaceAspectRatio = worldSpaceHalfWidth / worldSpaceHalfHeight;

// Screen space depends on space in window that is available to us. Although, we will
// keep the aspect ratio the same as world space, so we don't stretch, squeeze or
// change the visible area.
constexpr float screenSpaceAspectRatio = worldSpaceAspectRatio;
