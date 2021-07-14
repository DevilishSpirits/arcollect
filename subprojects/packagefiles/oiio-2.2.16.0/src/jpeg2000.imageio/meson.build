dep_openjpg2 = dependency('libopenjp2', required: get_option('enable_jpeg2000'))
if dep_openjpg2.found()
	oiio_plugins += declare_dependency(sources: files('jpeg2000input.cpp','jpeg2000input.cpp'), dependencies: dep_openjpg2, compile_args: '-DUSE_OPENJPEG=1')
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_JPEG2000=1')
endif
