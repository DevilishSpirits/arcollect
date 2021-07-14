# Note: OIIO has a hard dependency on libtiff even if the plugin is disabled
if not get_option('enable_tiff').disabled()
	oiio_plugins += declare_dependency(sources: files('tiffinput.cpp','tiffoutput.cpp'))
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_TIFF=1')
endif
