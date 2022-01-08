project('boost', 'cpp')
cpp = meson.get_compiler('cpp')
add_project_arguments('-DBOOST_ALL_STATIC_LINK', language: 'cpp')
boost_pub_args = [
]
# Enable modules
boost_with = get_option('boost_with')

if ('filesystem' in boost_with) and 'atomic' not in boost_with
	boost_with += 'atomic'
endif

boost_inc = include_directories('.')
boost_libs = []
foreach module: boost_with
	subdir('libs/'+module)
endforeach

boost_ver = cpp.get_define('BOOST_LIB_VERSION', include_directories: boost_inc, prefix: '#include <boost/version.hpp>').strip('"').replace('_','.')
boost_dep = declare_dependency(link_whole: boost_libs, include_directories: boost_inc, compile_args: boost_pub_args, version: boost_ver)
