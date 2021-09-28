offline_tests = files(
)
online_tests = files(
	'artstation.com.json',
	'e621.net.json',
	'furaffinity.net.json',
)
online_nsfw_tests = files(
	'e621.net-nsfw.json',
	'furaffinity.net-nsfw.json',
)

tests_env = environment({
	'ARCOLLECT_DEBUG': 'all',
	'ARCOLLECT_WEBEXT_ADDER_PATH': webext_adder_exe.full_path(),
})
tests_deps = [
	webext_adder_exe,
]

# Generate payloads externally (useful to test the adder in gdb)
make_test_payload_prog = find_program('make_test_payload.py')
#offline_tests_payload = custom_target('webext-adder-offline-test-payload', output: 'offline-payload', command: [make_test_payload_prog, offline_tests], build_by_default: false, capture: true)
online_tests_payload = custom_target('webext-adder-online-test-payload', output: 'online-payload', command: [make_test_payload_prog, online_tests], build_by_default: false, capture: true)
online_nsfw_tests_payload = custom_target('webext-adder-online-nsfw-test-payload', output: 'online-nsfw-payload', command: [make_test_payload_prog, online_nsfw_tests], build_by_default: false, capture: true)

webext_adder_test_prog = find_program('webext-adder-test.py')
#test('webext-adder-offline',webext_adder_test_prog, args: [meson.current_build_dir()/'tests-offline', offline_tests_payload, offline_tests], env: tests_env, depends: webext_adder_exe, protocol: 'tap', timeout: 600)
if get_option('tests_online')
	test('webext-adder-online',webext_adder_test_prog, args: [meson.current_build_dir()/'tests-online', online_tests_payload, online_tests], env: tests_env, depends: webext_adder_exe, protocol: 'tap', timeout: 600)
	if get_option('tests_nsfw')
		test('webext-adder-online-nsfw',webext_adder_test_prog, args: [meson.current_build_dir()/'tests-online-nsfw', online_nsfw_tests_payload, online_nsfw_tests], env: tests_env, depends: webext_adder_exe, protocol: 'tap', timeout: 600)
	endif
endif
