build_dir="$1"
if [ -z "$build_dir" ]; then
    echo "Specify build dir"
    exit 1
fi

pushd "$build_dir"
cmake --build . --target cmag
popd

rm -rf test/acceptance/with_subdirs/build
mkdir -p test/acceptance/with_subdirs/build
"$build_dir"/bin/Debug/cmag -p project -e MY_PROP cmake -S test/acceptance/with_subdirs -B test/acceptance/with_subdirs/build
"$build_dir"/bin/cmag       -p project -e MY_PROP cmake -S test/acceptance/with_subdirs -B test/acceptance/with_subdirs/build

echo
echo "----------------------- Configs:"
cat test/acceptance/with_subdirs/build/project.cmag-configs

echo "----------------------- Globals"
cat test/acceptance/with_subdirs/build/project.cmag-globals

echo "----------------------- Default config:"
cat test/acceptance/with_subdirs/build/project_Default.cmag-targets

echo "----------------------- Debug config:"
cat test/acceptance/with_subdirs/build/project_Debug.cmag-targets

echo "----------------------- Release config:"
cat test/acceptance/with_subdirs/build/project_Release.cmag-targets
