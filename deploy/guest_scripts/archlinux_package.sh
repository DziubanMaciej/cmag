#!/bin/sh

fail() {
    echo "ERROR: failed to $@" >&2
    exit 1
}

cmag_version="$1"
if [ -z "$cmag_version" ]; then
    fail "validate version"
fi
ssh_key_name="$2"
if [ -z "ssh_key_name" ]; then
    fail "validate ssh key path"
fi
ssh_key_path="$(realpath "$ssh_key_name")"
cd ~/workspace/package || fail "enter package directory"

# Generate .SRCINFO
makepkg --printsrcinfo > .SRCINFO

# Initialize git state
rm -rf .git
git -c init.defaultBranch=master init || fail "init git repo"
git remote add origin ssh://aur@aur.archlinux.org/cmag.git || fail "setup git remote"
git config --local user.name MaciejDziuban || fail "setup git user name"
git config --local user.email dziuban.maciej@gmail.com || fail "setup git user email"

# Fetch existing package code
export GIT_SSH_COMMAND="ssh -i $ssh_key_path -o IdentitiesOnly=yes"
git fetch origin || fail "fetch"

# Create a new commit for this release
git add PKGBUILD || fail "add PKGBUILD to git"
git add .SRCINFO || fail "add .SRCINFO to git"
git commit -am "Version $cmag_version" || fail "create git commit"

# Push to AUR
git push --set-upstream origin master || fail "push to AUR"
