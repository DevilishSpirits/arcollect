dep_jpeg = dependency('libjpeg', required: get_option('enable_jpeg'))
summary('jpeg', dep_jpeg.found(), bool_yn: true, section: 'ImageIO plugins')
if dep_jpeg.found()
	oiio_plugins += declare_dependency(sources: files('jpeginput.cpp','jpegoutput.cpp'), dependencies: dep_jpeg)
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_JPEG=1')
endif
