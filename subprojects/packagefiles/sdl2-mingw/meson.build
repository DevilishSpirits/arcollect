project('sdl2', 'c', version : '2.0.14', license : 'zlib')

archdir = meson.project_source_root() / host_machine.cpu()+'-w64-mingw32'

# Perform static linking
link_args = [
	archdir/'lib/libSDL2.a',
	archdir/'lib/libSDL2main.a',
	'-lmingw32',
	'-Wl,--no-undefined',
	'-Wl,--dynamicbase',
	'-Wl,--nxcompat',
	'-lm',
	'-ldinput8',
	'-ldxguid',
	'-ldxerr8',
	'-luser32',
	'-lgdi32',
	'-lwinmm',
	'-limm32',
	'-lole32',
	'-loleaut32',
	'-lshell32',
	'-lsetupapi',
	'-lversion',
	'-luuid',
	'-static-libgcc',
]

if host_machine.cpu() == 'x86_64'
	link_args += '-Wl,--high-entropy-va'
endif

sdl2_dep = declare_dependency(
	include_directories: include_directories(archdir / 'include/SDL2'),
	link_args: link_args,
)
