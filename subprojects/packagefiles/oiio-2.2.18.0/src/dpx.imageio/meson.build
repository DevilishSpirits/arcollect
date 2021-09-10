dep_openexr = dependency('OpenEXR', required: get_option('enable_dpx'))
summary('dpx', dep_openexr.found(), bool_yn: true, section: 'ImageIO plugins')
if dep_openexr.found()
	oiio_plugins += declare_dependency(sources: files(
		'dpxinput.cpp',
		'dpxoutput.cpp',
		'libdpx/DPX.cpp',
		'libdpx/OutStream.cpp',
		'libdpx/RunLengthEncoding.cpp',
		'libdpx/Codec.cpp',
		'libdpx/Reader.cpp',
		'libdpx/Writer.cpp',
		'libdpx/DPXHeader.cpp',
		'libdpx/ElementReadStream.cpp',
		'libdpx/InStream.cpp',
		'libdpx/DPXColorConverter.cpp',
	),dependencies: dep_openexr)
else
	oiio_plugins += declare_dependency(compile_args: '-DDISABLE_DPX=1')
endif
