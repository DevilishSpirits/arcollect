offline_tests = files(
)
online_tests = files(
	'artstation.com.json',
	'e621.net.json',
	'furaffinity.net.json',
	'tumbex.com.json',
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

nst_json_test_suite = files(
	# From https://github.com/nst/JSONTestSuite (MIT license)
	'nst_JSONTestSuite/y_array_arraysWithSpaces.json',
	'nst_JSONTestSuite/y_array_empty.json',
	'nst_JSONTestSuite/y_array_empty-string.json',
	'nst_JSONTestSuite/y_array_ending_with_newline.json',
	'nst_JSONTestSuite/y_array_false.json',
	'nst_JSONTestSuite/y_array_heterogeneous.json',
	'nst_JSONTestSuite/y_array_null.json',
	'nst_JSONTestSuite/y_array_with_1_and_newline.json',
	'nst_JSONTestSuite/y_array_with_leading_space.json',
	'nst_JSONTestSuite/y_array_with_several_null.json',
	'nst_JSONTestSuite/y_array_with_trailing_space.json',
	'nst_JSONTestSuite/y_number_0e+1.json',
	'nst_JSONTestSuite/y_number_0e1.json',
	'nst_JSONTestSuite/y_number_after_space.json',
	'nst_JSONTestSuite/y_number_double_close_to_zero.json',
	'nst_JSONTestSuite/y_number_int_with_exp.json',
	'nst_JSONTestSuite/y_number.json',
	'nst_JSONTestSuite/y_number_minus_zero.json',
	'nst_JSONTestSuite/y_number_negative_int.json',
	'nst_JSONTestSuite/y_number_negative_one.json',
	'nst_JSONTestSuite/y_number_negative_zero.json',
	'nst_JSONTestSuite/y_number_real_capital_e.json',
	'nst_JSONTestSuite/y_number_real_capital_e_neg_exp.json',
	'nst_JSONTestSuite/y_number_real_capital_e_pos_exp.json',
	'nst_JSONTestSuite/y_number_real_exponent.json',
	'nst_JSONTestSuite/y_number_real_fraction_exponent.json',
	'nst_JSONTestSuite/y_number_real_neg_exp.json',
	'nst_JSONTestSuite/y_number_real_pos_exponent.json',
	'nst_JSONTestSuite/y_number_simple_int.json',
	'nst_JSONTestSuite/y_number_simple_real.json',
	'nst_JSONTestSuite/y_object_basic.json',
	'nst_JSONTestSuite/y_object_duplicated_key_and_value.json',
	'nst_JSONTestSuite/y_object_duplicated_key.json',
	'nst_JSONTestSuite/y_object_empty.json',
	'nst_JSONTestSuite/y_object_empty_key.json',
	'nst_JSONTestSuite/y_object_escaped_null_in_key.json',
	'nst_JSONTestSuite/y_object_extreme_numbers.json',
	'nst_JSONTestSuite/y_object.json',
	'nst_JSONTestSuite/y_object_long_strings.json',
	'nst_JSONTestSuite/y_object_simple.json',
	'nst_JSONTestSuite/y_object_string_unicode.json',
	'nst_JSONTestSuite/y_object_with_newlines.json',
	'nst_JSONTestSuite/y_string_1_2_3_bytes_UTF-8_sequences.json',
	'nst_JSONTestSuite/y_string_accepted_surrogate_pair.json',
	'nst_JSONTestSuite/y_string_accepted_surrogate_pairs.json',
	'nst_JSONTestSuite/y_string_allowed_escapes.json',
	'nst_JSONTestSuite/y_string_backslash_and_u_escaped_zero.json',
	'nst_JSONTestSuite/y_string_backslash_doublequotes.json',
	'nst_JSONTestSuite/y_string_comments.json',
	'nst_JSONTestSuite/y_string_double_escape_a.json',
	'nst_JSONTestSuite/y_string_double_escape_n.json',
	'nst_JSONTestSuite/y_string_escaped_control_character.json',
	'nst_JSONTestSuite/y_string_escaped_noncharacter.json',
	'nst_JSONTestSuite/y_string_in_array.json',
	'nst_JSONTestSuite/y_string_in_array_with_leading_space.json',
	'nst_JSONTestSuite/y_string_last_surrogates_1_and_2.json',
	'nst_JSONTestSuite/y_string_nbsp_uescaped.json',
	'nst_JSONTestSuite/y_string_nonCharacterInUTF-8_U+10FFFF.json',
	'nst_JSONTestSuite/y_string_nonCharacterInUTF-8_U+FFFF.json',
	'nst_JSONTestSuite/y_string_null_escape.json',
	'nst_JSONTestSuite/y_string_one-byte-utf-8.json',
	'nst_JSONTestSuite/y_string_pi.json',
	'nst_JSONTestSuite/y_string_reservedCharacterInUTF-8_U+1BFFF.json',
	'nst_JSONTestSuite/y_string_simple_ascii.json',
	'nst_JSONTestSuite/y_string_space.json',
	'nst_JSONTestSuite/y_string_surrogates_U+1D11E_MUSICAL_SYMBOL_G_CLEF.json',
	'nst_JSONTestSuite/y_string_three-byte-utf-8.json',
	'nst_JSONTestSuite/y_string_two-byte-utf-8.json',
	'nst_JSONTestSuite/y_string_u+2028_line_sep.json',
	'nst_JSONTestSuite/y_string_u+2029_par_sep.json',
	'nst_JSONTestSuite/y_string_uescaped_newline.json',
	'nst_JSONTestSuite/y_string_uEscape.json',
	'nst_JSONTestSuite/y_string_unescaped_char_delete.json',
	'nst_JSONTestSuite/y_string_unicode_2.json',
	'nst_JSONTestSuite/y_string_unicodeEscapedBackslash.json',
	'nst_JSONTestSuite/y_string_unicode_escaped_double_quote.json',
	'nst_JSONTestSuite/y_string_unicode.json',
	'nst_JSONTestSuite/y_string_unicode_U+10FFFE_nonchar.json',
	'nst_JSONTestSuite/y_string_unicode_U+1FFFE_nonchar.json',
	'nst_JSONTestSuite/y_string_unicode_U+200B_ZERO_WIDTH_SPACE.json',
	'nst_JSONTestSuite/y_string_unicode_U+2064_invisible_plus.json',
	'nst_JSONTestSuite/y_string_unicode_U+FDD0_nonchar.json',
	'nst_JSONTestSuite/y_string_unicode_U+FFFE_nonchar.json',
	'nst_JSONTestSuite/y_string_utf8.json',
	'nst_JSONTestSuite/y_string_with_del_character.json',
	'nst_JSONTestSuite/y_structure_lonely_false.json',
	'nst_JSONTestSuite/y_structure_lonely_int.json',
	'nst_JSONTestSuite/y_structure_lonely_negative_real.json',
	'nst_JSONTestSuite/y_structure_lonely_null.json',
	'nst_JSONTestSuite/y_structure_lonely_string.json',
	'nst_JSONTestSuite/y_structure_lonely_true.json',
	'nst_JSONTestSuite/y_structure_string_empty.json',
	'nst_JSONTestSuite/y_structure_trailing_newline.json',
	'nst_JSONTestSuite/y_structure_true_in_array.json',
	'nst_JSONTestSuite/y_structure_whitespace_array.json'
)
test_wtf_json_parser_exe = executable('test_wtf_json_parser', 'test_wtf_json_parser.cpp', build_by_default: false)
test_wtf_json_parser_py = files('test_wtf_json_parser.py')
test('wtf-json-parser',test_wtf_json_parser_py, args: [test_wtf_json_parser_exe,offline_tests,online_tests,online_nsfw_tests,nst_json_test_suite], protocol: 'tap')
