#!/bin/sh

cmag_commit="$1"
cmag_version="$2"

fail() {
    echo "ERROR: failed to $@" >&2
    exit 1
}

# Prepare package directory
cd ~/workspace
mkdir -p package || fail "create package directory"
cd package

# Create control file
mkdir -p DEBIAN || fail "create DEBIAN directory"
cat <<EOT > DEBIAN/control
Package: cmag
Version: $cmag_version
Architecture: amd64
Maintainer: Maciej Dziuban <dziuban.maciej@gmail.com>
Installed-Size: 6
Section: devel
Priority: optional
Homepage: https://github.com/DziubanMaciej/cmag
Description: Interactive analyzer and browser for CMake build systems
EOT

# Copy binaries
mkdir -p bin || fail "create bin directory"
cp ~/workspace/cmag/build/bin/cmag bin/cmag || fail "copy cmag binary"
cp ~/workspace/cmag/build/bin/cmag bin/cmag_browser || fail "copy cmag_browser binary"

# Build .deb file
dpkg-deb --build . ~/workspace || fail "create .deb file"