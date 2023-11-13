#!/bin/sh

build_dir="$1"
if [ -z "$build_dir" ]; then
    echo "Specify build dir"
    exit 1
fi

pushd "$build_dir"
cmake --build . --target cmag cmag-browser || exit 1
popd

bin_dir="$build_dir/bin"
"$bin_dir"/cmag -p self_run cmake -S . -B "$build_dir" || exit 1
"$bin_dir"/cmag-browser "$build_dir"/self_run.cmag-project || exit 1
