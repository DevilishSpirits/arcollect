# Arcollect override
#
# curl override CMAKE_MODULE_PATH and hide out FindBrotli module.
# We shim to our version there.
include("${ARCOLLECT_CMAKE_MODULE_PATH}/FindBrotli.cmake")
