webext_zip_cmds = [
	find_program('build_zip.py', native: true),
	meson.current_build_dir(),
	'@OUTPUT@'
]
webext_zip_depend_files = []
webext_srcs = [
	'manifest.json',
	'background.js',
]
webext_content_scripts = [
	'common.js',
	'artstation.com.js',
	'deviantart.com.js',
	'e621.net.js',
	'furaffinity.net.js',
]
subdir('content-scripts')
foreach file: webext_srcs
	webext_zip_cmds += file
	webext_zip_depend_files += configure_file(output: file, input: file, configuration: config_h)
endforeach

webextension_target = custom_target('arcollect.zip', output : 'arcollect.zip', command : webext_zip_cmds, build_by_default: true)

# Test with web-ext lint
prog_web_ext = find_program('web-ext', required: false)
prog_unzip   = find_program('unzip', required: false)
if prog_web_ext.found() and prog_unzip.found()
	test('web-ext-lint', files('web-ext-lint.sh'), args: [webextension_target, meson.project_build_root(), prog_unzip.full_path(), prog_web_ext.full_path()], suite: ['webextension'])
else
	warning('"web-ext" or "unzip" missing. Will not check linting.')
endif
if enable_native_progs and get_option('tests_browser')
	subdir('tests')
endif
