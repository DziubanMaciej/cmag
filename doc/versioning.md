The version of *cmag* is composed of three integer components. They are referred to by their index, e.g. *component0*. Both *cmag* binaries (the dumper and the browser) are versioned in tandem. They will always have the same version within the same build. Version components are defined as follows:
- *component0* - incremented whenever *cmag* undergoes big, fundamental changes. Very rare.
- *component1* - incremented whenever the structure of `.cmag-project` files changes and compatibility is broken.
- *component2* - incremented whenever *cmag* gains new features, that are backward-compatible.

It is intentional to **not** call the components *major*, *minor* and *patch*, because this is **not** [semantic versioning](https://semver.org/).

In order to check the version of *cmag* or *cmag_browser* binary, use the `-v` argument.

The dumper writes its version to the `.cmag-project` files, which lets the browser identify the version of browsed file. Browsing project files with incompatible versions is unsupported and not guaranteed to work. Project file version is incompatible with browser's version when it has a different *component0* or *component1*.  
