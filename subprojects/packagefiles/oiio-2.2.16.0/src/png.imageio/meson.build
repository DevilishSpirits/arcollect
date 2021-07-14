dep_png = dependency('libpng', required: get_option('enable_png'))
if dep_png.found()
	oiio_plugins += declare_dependency(sources: files('pnginput.cpp','pngoutput.cpp'), dependencies: dep_png)
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_PNG=1')
endif
