#!/bin/sh

fail() {
    echo "ERROR: failed to $@" >&2
    exit 1
}

cmag_commit="$1"
cmag_version="$2"
if [ -z "cmag_commit" ]; then
    fail "validate commit"
fi
if [ -z "$cmag_version" ]; then
    fail "validate version"
fi
clean_run=1

# Enter package directory
if [ "$clean_run" = "1" ]; then
    rm -rf package || fail "cleanup package directory"
fi
mkdir -p package
cd ~/workspace/package || fail "enter package directory"

# Patch package version in PKGBUILD
sed -E "s/^pkgver=.*/pkgver=$cmag_version/g; s/^_tag=.*/_tag=$cmag_commit # git rev-parse v$cmag_version/g" ~/workspace/PKGBUILD > PKGBUILD || fail "patch the PKGBUILD file"

# Build package
makepkg_args=""
if [ "$clean_run" != "1" ]; then
    makepkg_args="$makepkg_args --noextract --force"
fi
makepkg $makepkg_args || fail "build package"

# Self run
cd src/cmag/build || fail "enter source directory"
bin/cmag cmake .. || fail "perform a self cmag run"

# Verify version
actual_cmag_version="$(bin/cmag -v)" || fail "get cmag version"
if [ "$actual_cmag_version" != "$cmag_version" ]; then
    fail "validate version. Expected \"$cmag_version\", got \"$actual_cmag_version\""
fi
