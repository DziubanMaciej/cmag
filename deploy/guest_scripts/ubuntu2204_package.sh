#!/bin/sh

fail() {
    echo "ERROR: failed to $@" >&2
    exit 1
}

cmag_version="$1"
if [ -z "$cmag_version" ]; then
    fail "validate version"
fi

# Enter package directory
mkdir package
cd ~/workspace/package || fail "enter package directory"

# Prepare the source
archive_name="cmag_$cmag_version.orig.tar.gz"
source_dir_name="cmag-$cmag_version"
~/workspace/unix_prepare_source_tarball.sh "$cmag_version" "$archive_name" || fail "create source tarball"
tar -xzf "$archive_name" || fail "untar source tarball"
mv "source" "$source_dir_name" || fail "rename source directory"
cd "$source_dir_name" || fail "enter source directory"

# Copy package metadata
cp -R ~/workspace/debian . || fail "copy package metadata to source directory"

# Build .deb
debuild || fail "execute debuild"
