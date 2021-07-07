# FIXME Convert the CMake to Meson to avoid this hack
message("Arcollect: Disable ZLIB to fix Windows build.")
SET(ZLIB_FOUND FALSE)
