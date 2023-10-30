#!/bin/bash

install_clang_format() {
    which clang-format && {
        echo "clang-format already installed"
        return 0
    }

    which apt-get
    if [ $? != 0 ]; then
        if [ `dpkg --list | grep " clang-format " | wc -l` -eq 0 ]; then
            echo "Installing clang-format with apt-get"
            sudo apt-get install clang-format || return 1
        fi
    fi

    which clang-format
    return $?
}

install_clang_format || {
    echo "ERROR: Could not install clang-format"
    exit 1
}

root_dir=`echo ${BASH_SOURCE[0]} | xargs realpath | xargs dirname | xargs dirname | xargs dirname` # TODO find better way to go up three levels? Adding ".." is ugly.
find .                                                                 \
    -type f                                                            \
    \( -name "*.cpp" -o -name "*.c" -o -name "*.h" -o -name "*.hpp" \) \
    -not -path "./third_party/*"                                       \
    -not -path "./build*/*"                                            \
    -not -path "./cmake-build-*/*"                                     \
    -print                                                             \
| xargs clang-format -i --verbose
