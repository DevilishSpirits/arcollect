project('sdl2-image', 'c', version : '2.0.5', license : 'zlib')

archdir = meson.project_source_root() / host_machine.cpu()+'-w64-mingw32'

sdl2_image_dep = declare_dependency(
	include_directories: include_directories(archdir / 'include/SDL2'),
	link_args: ['-I'+archdir / 'lib', '-llibSDL2_image.a'],
)
