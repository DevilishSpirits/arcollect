project('sdl2', 'c', version : '2.0.14', license : 'zlib')

archdir = meson.project_source_root() / host_machine.cpu()+'-w64-mingw32'

sdl2_dep = declare_dependency(
	include_directories: include_directories(archdir / 'include/SDL2'),
	link_args: ['-L'+archdir / 'lib', '-lSDL2', '-lSDL2main', '-lmingw32'],
)
