project('FMT', 'cpp', version: '8.0.1')
incdirs = include_directories('include')
fmt = static_library('fmt', 'src/format.cc', 'src/os.cc', include_directories: incdirs)
fmt_dep = declare_dependency(link_with: fmt, include_directories: incdirs)
