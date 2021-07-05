webext_src = [
	configure_file(output: 'manifest.json', input: 'manifest.json.in', configuration: config_h),
	'background.js',
	'content-scripts/common.js',
	'content-scripts/artstation.com.js',
	'content-scripts/deviantart.com.js',
	'content-scripts/e621.net.js',
	'content-scripts/furaffinity.net.js',
]

webextension_target = custom_target('arcollect.zip', output : 'arcollect.zip', input : webext_src, command : [files('build.sh'), '@CURRENT_SOURCE_DIR@', meson.current_build_dir(), prog_zip, '@OUTPUT@', '@INPUT@'], build_by_default: true)

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
