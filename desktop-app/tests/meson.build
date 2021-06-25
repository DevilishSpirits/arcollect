desktop_app_test_env = environment({
	'XDG_CONFIG_HOME': meson.current_build_dir(),
	'XDG_DATA_HOME': meson.current_build_dir(),
})

test('test-config', executable('test-config', 'test-config.cpp', dependencies: desktop_app_dep), env: desktop_app_test_env)
