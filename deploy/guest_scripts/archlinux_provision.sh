#!/bin/sh

fail() {
    echo "ERROR: failed to $@" >&2
    exit 1
}

sudo pacman -Syy --noconfirm git cmake base-devel xorg mesa || fail "install dependencies"
