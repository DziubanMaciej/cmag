# cmag

Project *cmag* is an interactive browser for the *CMake* build system. It analyzes a *CMake* project, extracts information about it into a custom *.cmag-project* file and then displays it in a GUI application. It doesn't require any integration - any project can be viewed in *cmag*. Currently supported systems are Windows and Linux.



# Usage
To generate *cmag* project file, simply prepend the name of *cmag* binary to your *cmake* command. For example, if you normally run:
```
cmake .. -DCMAKE_BUILD_TYPE=Release
```

run the following command instead:
```
cmag cmake -DCMAKE_BUILD_TYPE=Release
```

This command prepares your build system with *CMake* and generates a *.cmag-project* file. This file can be opened with GUI component of *cmag* called *cmag_browser*.
```
cmag_browser project.cmag-project
```



# Browser



# Installation
Currently, *cmag* is not distributed in any open repositories.

# From source
Compilation requirements: `git`, `graphviz`, `cmake` and a C++ compiler. Run the following commands:
```
git submodule init
git submodule upate
mkdir build
cd build
cmake -B .
cmake --build .
cmake --install .
```



# Roadmap
This is a current list of items left to do in this project. It may shrink and grow over time, but should be kept relatively up to date.
- project wide
  - [ ] Add some presentation and screenshots of the browser in this README.
  - [ ] Add cmag version. Store it in project files, return it from binaries and validate it when loading projects.
- cmag
  - [ ] Explore if handling targets created in [included](https://cmake.org/cmake/help/latest/command/include.html) files is possible. Currently, we make it look as if they were created in CMakeLists.txt itself.
  - [ ] Explore if handling CMake `find_package()` is possible. Currently, we ignore these targets. We could probably get them from [cmake-file-api](https://cmake.org/cmake/help/latest/manual/cmake-file-api.7.html), but that would require an additional pass.
  - [ ] Break the dependency from graphviz. Graph layout should be handled in a more robust way.
  - [ ] Add textual errors to parsers. We often get "failed parsing X" without any more helpful information.
  - [ ] Implement `--merge` argument. For single config generators, such as `Unix Makefiles` we should be able to merge outputs of two `cmag` analyses into a one project to be able to compare different configs.
  - [ ] Cleanup intermediate files. Only leave *.cmag-project* file. Add some cmdline option to leave them for debugging purposes.
- cmag_browser
  - [ ] Use a custom font.
  - [ ] Add a tooltip on target hovering.
  - [ ] Make ListFileTab look nicer with some icons. To achieve this, some [ImGui font magic](https://github.com/ocornut/imgui/blob/master/docs/FONTS.md) must be used.
  - [ ] Hide debug controls from the UI. Add some option to show it. CMake option? Cmdline arg? Env variable?
  - [ ] Make sure we don't throw exceptions when something is wrong with the file. End gracefully.
  - [ ] Add warning in target folders tab when [USE_FOLDERS](https://cmake.org/cmake/help/latest/prop_gbl/USE_FOLDERS.html) is OFF. See also [CMP0143](https://cmake.org/cmake/help/latest/policy/CMP0143.html).
