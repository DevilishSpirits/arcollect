# CMake injection

This folder override custom CMake dependencies to inject our locally sourced
Meson ones like zlib into CMake powered dependencies.

You have to defines the `cmake_overrides_path` to all `<PackageName>_ROOT`.
You can also `add_cmake_defines(cmake_overrides_defines)` to set all defines.
