messages_json = custom_target('webextension-messages.json', command: [
	executable('generate-messages','generate-messages.cpp', dependencies: [i18n_deps['common'],i18n_deps['webextension']], build_by_default: false)
], output: 'messages.json', capture: true)

webext_zip_cmds = [
	find_program('build_zip.py', native: true),
	meson.current_build_dir(),
	messages_json,
	'@OUTPUT@'
]
webext_zip_depend_files = []
webext_zip_depends      = []
webext_conf_srcs = [
	'manifest.json',
]
webext_srcs = [
	'background.js',
	'fetch.js',
	'twitter.js',
]
webext_content_scripts = [
	'common.js',
	'SaveControlHelper.js',
	'artstation.com.js',
	'deviantart.com.js',
	'e621.net.js',
	'e621.net-keyboard_shortcuts.js',
	'furaffinity.net.js',
	'knowyourmeme.com.js',
	'patreon.com.js',
	'pixiv.net.js',
	'twitter.com.js',
	'tumblr.com-shared.js',
	'tumbex.com.js',
	'xkcd.com.js',
]
python_copy_command = [find_program('python3'),'-c','import shutil; import sys; shutil.copyfile(sys.argv[1],sys.argv[2])','@INPUT@','@OUTPUT@']
subdir('content-scripts')
foreach file: webext_srcs
	webext_zip_cmds += file
	webext_zip_depends += custom_target(file, command: python_copy_command, input: file, output: file)
endforeach
foreach file: webext_conf_srcs
	webext_zip_cmds += file
	webext_zip_depend_files += configure_file(output: file, input: file, configuration: config_h)
endforeach

webextension_target = custom_target('arcollect.zip', output : 'arcollect.zip', command : webext_zip_cmds, depend_files: webext_zip_depend_files, depends: webext_zip_depends, build_by_default: true)

# Test with web-ext lint
prog_web_ext = find_program('web-ext', required: false, version: '<7.5')
prog_unzip   = find_program('unzip', required: false)
if prog_web_ext.found() and prog_unzip.found()
	test('web-ext-lint', files('web-ext-lint.sh'), args: [webextension_target, meson.project_build_root(), prog_unzip.full_path(), prog_web_ext.full_path()], suite: ['webextension'])
else
	warning('"web-ext" or "unzip" missing. Will not check linting.')
endif
if enable_native_progs and get_option('tests_browser')
	subdir('tests')
endif
