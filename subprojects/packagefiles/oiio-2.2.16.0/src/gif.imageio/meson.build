if get_option('enable_gif').disabled()
	dep_giflib = disabler()
else
	dep_giflib = cpp.find_library('libgif', required: false)
	if not dep_giflib.found()
		dep_giflib = dependency('giflib')
	endif
endif
if dep_giflib.found()
	oiio_plugins += declare_dependency(sources: files(
		'gifinput.cpp',
		'gifoutput.cpp',
	),dependencies: dep_giflib, compile_args: '-DUSE_GIF=1')
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_GIF=1')
endif
