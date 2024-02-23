#!/bin/sh

fail() {
    echo "ERROR: failed to $@" >&2
    exit 1
}

cmag_version="$1"
cmag_archive_name="$2"
if [ -z "$cmag_version" -o -z "$cmag_archive_name" ]; then
    fail "validate version"
fi
cmag_commit="v$cmag_version"
cmag_dir="source"

# Clone the repository
rm -rf "$cmag_dir" || remove "$cmag_dir directory before cloning"
if ! [ -e "$cmag_dir" ]; then
    git clone https://github.com/DziubanMaciej/cmag "$cmag_dir" || fail "clone the repository"
    cd "$cmag_dir"
else
    cd "$cmag_dir"
    git fetch || fail "fetch the repository"
fi
git checkout $cmag_commit || fail "checkout $cmag_commit"
git submodule update --init --recursive || fail "update submodules"

# Cleanup from source control files
find . -name ".git" | xargs rm -rf || fail "cleanup files"

# Pack into tarball and cleanup
cd ..
tar -czvf "$cmag_archive_name" "$cmag_dir" || fail "pack to $cmag_cmag_archive_name"
rm -rf "$cmag_dir" || fail "remove $cmag_dir after packing"
