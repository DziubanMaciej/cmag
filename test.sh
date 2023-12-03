#!/bin/sh

build_dir="$1"
if [ -z "$build_dir" ]; then
    echo "Specify build dir"
    exit 1
fi

pushd "$build_dir"
cmake --build . --target cmag cmag_browser || exit 1
popd

bin_dir="$build_dir/bin/Debug"
"$bin_dir"/cmag -p self_run -g cmake -S . -B "$build_dir" || exit 1
