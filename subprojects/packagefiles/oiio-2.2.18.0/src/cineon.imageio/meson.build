if not get_option('enable_cineon').disabled()
	oiio_plugins += declare_dependency(sources: files(
		'cineoninput.cpp',
		'libcineon/Cineon.cpp',
		'libcineon/OutStream.cpp',
		'libcineon/Codec.cpp',
		'libcineon/Reader.cpp',
		'libcineon/Writer.cpp',
		'libcineon/CineonHeader.cpp',
		'libcineon/ElementReadStream.cpp',
		'libcineon/InStream.cpp',
	))
	summary('cineon', true , bool_yn: true, section: 'ImageIO plugins')
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_CINEON=1')
	summary('cineon', false, bool_yn: true, section: 'ImageIO plugins')
endif
