#!/bin/sh

cmag_commit="$1"
expected_cmag_version="$2"

fail() {
    echo "ERROR: failed to $@" >&2
    exit 1
}

# Clone the repository
if ! [ -e cmag ]; then
    git clone https://github.com/DziubanMaciej/cmag
    cd cmag
else
    cd cmag
    git fetch
fi
git checkout $cmag_commit || fail "checkout $cmag_commit"
git submodule update --init --recursive || fail "update submodules"

# Configure CMake
mkdir build -p
cd build
cmake ..                       \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAG_BUILD_TESTS=OFF     \
    -DCMAG_BUILD_BROWSER=ON    \
    -DGLFW_BUILD_X11=ON        \
    -DGLFW_BUILD_WAYLAND=OFF   || fail "run CMake"

# Compile
cmake --build . --config Release -- -j $(nproc) || fail "compile cmag"

# Self run
bin/cmag cmake .. || fail "perform a self cmag run"

# Verify version
actual_cmag_version="$(bin/cmag -v)" || fail "get cmag version"
if [ "$actual_cmag_version" != "$expected_cmag_version" ]; then
    fail "validate version. Expected \"$expected_cmag_version\", got \"$actual_cmag_version\""
fi
