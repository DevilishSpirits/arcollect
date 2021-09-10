# TODO Search for system libsquish
if not get_option('enable_dds').disabled()
	oiio_plugins += declare_dependency(sources: files(
		'ddsinput.cpp',
		'squish/alpha.cpp',
		'squish/clusterfit.cpp',
		'squish/colourblock.cpp',
		'squish/colourfit.cpp',
		'squish/colourset.cpp',
		'squish/maths.cpp',
		'squish/rangefit.cpp',
		'squish/singlecolourfit.cpp',
		'squish/squish.cpp',
	), include_directories: include_directories('squish'))
	summary('dds', true, bool_yn: true, section: 'ImageIO plugins')
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_DDS=1')
	summary('dds', false, bool_yn: true, section: 'ImageIO plugins')
endif
