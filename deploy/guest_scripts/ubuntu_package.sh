#!/bin/sh

fail() {
    echo "ERROR: failed to $@" >&2
    exit 1
}

cmag_version="$1"
distros="$2"
clean_run=1
do_upload=1
if [ -z "$cmag_version" ]; then
    fail "validate version"
fi
if [ -z "$distros" ]; then
    fail "validate distros"
fi

# Enter package directory
if [ "$clean_run" = "1" ]; then
    rm -rf package || fail "cleanup package directory"
fi
mkdir -p package
cd ~/workspace/package || fail "enter package directory"

# Prepare the source directory
source_dir_name="cmag-$cmag_version"
if [ "$clean_run" = "1" ]; then
    archive_name="cmag_$cmag_version.orig.tar.gz"
    ~/workspace/ubuntu_prepare_source_tarball.sh "$cmag_version" "$archive_name" || fail "create source tarball"
    tar -xzf "$archive_name" || fail "untar source tarball"
    mv "source" "$source_dir_name" || fail "rename source directory"
fi
cd "$source_dir_name" || fail "enter source directory: $source_dir_name"

# Copy package metadata
cp -R ~/workspace/debian . || fail "copy package metadata to source directory"

# Create separate packages for all distros
for distro in $distros; do
    # Generate new changelog with added distro version
    echo "Running debuild for $distro"
    head -1 debian/changelog | sed "s/)/ubuntu~unstable1)/g" | sed "s/unstable/$distro/g"      > debian/changelog.tmp
    echo ""                                                                                   >> debian/changelog.tmp
    echo "  * Release for $distro"                                                            >> debian/changelog.tmp
    echo                                                                                      >> debian/changelog.tmp
    echo " -- Maciej Dziuban <dziuban.maciej@gmail.com>  $(date +"%a, %d %b %Y %H:%M:%S %z")" >> debian/changelog.tmp
    echo                                                                                      >> debian/changelog.tmp
    cat debian/changelog                                                                      >> debian/changelog.tmp
    mv debian/changelog.tmp debian/changelog || fail "create distro changelog"

    # Build source package
    debuild -S -sa -k'dziuban.maciej@gmail.com' || fail "execute debuild"
    echo ---------------------------------------------

    # Restore changelog
    cp ~/workspace/debian/changelog debian/changelog
done

# Upload all packages to Launchpad
if [ "$do_upload" = "1" ]; then
  find ~/workspace/package -maxdepth 1 -name "*_source.changes" | while read -r package_file ; do
      echo "Uploading $package_file"
      dput ppa:mdziuban/cmag "$package_file" || fail "upload $package_file"
      echo ""
  done
fi
