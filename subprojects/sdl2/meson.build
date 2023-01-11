# Thin wrapper around the CMake project
project('sdl2', 'c')

cc = meson.get_compiler('c')
cmake = import('cmake')
subproj_opts = cmake.subproject_options()
subproj_opts.add_cmake_defines({
	'BUILD_SHARED_LIBS': false,
	'SDL_AUDIO'        : false,
	'SDL_VIDEO'        : true,
	'SDL_RENDER'       : true,
	'SDL_EVENTS'       : true,
	'SDL_JOYSTICK'     : true,  # If disabled, cause link failures
	'SDL_HAPTIC'       : false,
	'SDL_LOCALE'       : true,
	'SDL_POWER'        : false,
	'SDL_THREADS'      : true,  # Require by some build-systems
	'SDL_TIMERS'       : true,  # For SDL_GetTicks(). TODO Switch to the C++ standard library
	'SDL_FILE'         : true,
	'SDL_LOADSO'       : true,
	'SDL_CPUINFO'      : false,
	'SDL_FILESYSTEM'   : false,
	'SDL_SENSOR'       : false,
	'SDL_STATIC_PIC'   : true,
	'WAYLAND_LIBDECOR' : false, # FIXME Add a hard depend on libdecor currently
})
dep_sdl2 = [
	cc.find_library('dl', required: false),
	cc.find_library('Imm32', required: false),
	cc.find_library('Setupapi', required: false),
	cc.find_library('Version', required: false),
	cc.find_library('Winmm', required: false),
]
subproj_opts.set_install(false)
subproj_sdl2 = cmake.subproject('sdl2-cmake', options: subproj_opts)
dep_sdl2 += subproj_sdl2.dependency('SDL2main').as_link_whole()
dep_sdl2_cmake = subproj_sdl2.dependency('SDL2-static')
sdl2_inc = subproj_sdl2.include_directories('SDL2-static')
sdl2_ver = []
foreach define: ['SDL_MAJOR_VERSION','SDL_MINOR_VERSION','SDL_PATCHLEVEL']
	sdl2_ver += cc.get_define(define, prefix: '#include <include/SDL_version.h>', include_directories: sdl2_inc)
endforeach
sdl2_dep = declare_dependency(dependencies: [dep_sdl2_cmake,dep_sdl2], version: '.'.join(sdl2_ver))
meson.override_dependency('sdl2',sdl2_dep)
