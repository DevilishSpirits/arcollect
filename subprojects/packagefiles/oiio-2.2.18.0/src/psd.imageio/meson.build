dep_jpeg = dependency('libjpeg', required: get_option('enable_psd'))
summary('psd', dep_jpeg.found(), bool_yn: true, section: 'ImageIO plugins')
if dep_jpeg.found()
	oiio_plugins += declare_dependency(sources: files(
		'psdinput.cpp',
		'jpeg_memory_src.cpp',
	),dependencies: dep_jpeg)
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_PSD=1')
endif
