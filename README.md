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
Compilation requirements: `git`, `cmake` (minimum 3.15.0) and a C++ compiler (C++17 required). Run the commands below. Note that the `install` command must be run with administrative privileges. To install to a custom location without administrative privileges, pass `-DCMAKE_INSTALL_PREFIX=<path>` to the first `cmake` command.
```
git submodule update --init --recursive
mkdir build
cd build
cmake ..
cmake --build .
cmake --install .
```
