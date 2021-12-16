# Simple roboto wrapper
project('roboto', 'cpp', version: '2.138')
roboto_src = [
	'Roboto-Light.ttf',
]
gen_roboto_exe = executable('gen-roboto-embed', 'gen-roboto-embed.cpp', install: false, build_by_default: false)
gen_roboto_target = custom_target('roboto-embed', input: roboto_src, command: [gen_roboto_exe, '@OUTPUT0@', '@OUTPUT1@', '@INPUT@'], output: ['roboto.cpp', 'arcollect-roboto.hpp'], install: false, build_by_default: false)
