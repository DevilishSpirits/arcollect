project('lcms2', 'c', version: '2.12', default_options: ['c_std=c99'])

incdirs = include_directories('include')
subdir('src')

lcms2_dep = declare_dependency(link_with: lcms2, include_directories: incdirs, version: meson.project_version())
