# Arcollect desktop application

This is the *Arcollect* application, the desktop viewer that is splitted in multiple directories.

* `art-reader` contain code to read various artworks
* `db` contain low-level database related code
* `gui` contain the user interface and most of the code
* `sdl2-hpp` contain a in-house C++ wrap of the SDL2
* `tests` contain tests (what would you expect?).
* `xdg` contain FreeDesktop specific code (enabled on Linux and FOSSy things with `-Duse_xdg=true`, auto-detection by default).

The real GUI main-loop is stored in [`gui/main.cpp`](gui/main.cpp). [`main.cpp`](main.cpp) is the default cross platform wrapper while [`main-xdg.cpp`](main-xdg.cpp) integrate a D-Bus main-loop.
