if not get_option('enable_bmp').disabled()
	oiio_plugins += declare_dependency(sources: files('bmpinput.cpp','bmpoutput.cpp','bmp_pvt.cpp'))
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_BMP=1')
endif
