# CMake hacking

This folder override custom CMake modules to inject our locally sourced Meson
dependencies like zlib into CMake libraries.

In Meson, ensure to set `CMAKE_MODULE_PATH` to `cmake_module_path` in CMake.
