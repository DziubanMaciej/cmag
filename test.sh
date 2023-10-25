pushd cmake-build-debug
make -j16 cmag
popd

rm -rf test/acceptance/with_subdirs/build
mkdir -p test/acceptance/with_subdirs/build
cmake-build-debug/cmag_exe/cmag cmake -S test/acceptance/with_subdirs -B test/acceptance/with_subdirs/build -DCMAG_PROJECT_NAME=project

echo
echo 'Configs:'
cat test/acceptance/with_subdirs/build/project.cmag-configs

echo
echo "Globals"
cat test/acceptance/with_subdirs/build/project.cmag-globals

echo
echo "Default config:"
cat test/acceptance/with_subdirs/build/project_Default.cmag-targets
