project('libpng', 'c', version : '1.6.37', license : 'libpng')

png_versions = meson.project_version().split('.')
png_major_version = png_versions[0]
png_minor_version = png_versions[1]
png_libname = 'png@0@@1@'.format(png_major_version, png_minor_version)

cc = meson.get_compiler('c')

include = include_directories('.')

if cc.get_define('__ANDROID__') != ''
  zlib_dep = cc.find_library('z')
else
  zlib_dep = dependency('zlib')
endif

c_args = []

libpng_deps = [
        zlib_dep,
        cc.find_library('m', required : false),
]

png_src = files([
        'png.c',
        'pngerror.c',
        'pngget.c',
        'pngmem.c',
        'pngpread.c',
        'pngread.c',
        'pngrio.c',
        'pngrtran.c',
        'pngrutil.c',
        'pngset.c',
        'pngtrans.c',
        'pngwio.c',
        'pngwrite.c',
        'pngwtran.c',
        'pngwutil.c'
])


if host_machine.cpu_family() == 'aarch64' or cc.get_define('__ARM_NEON') != ''
    png_src += files([
        'arm/arm_init.c',
        'arm/filter_neon_intrinsics.c',
        'arm/palette_neon_intrinsics.c',
        'arm/filter_neon.S'
    ])
    
    c_args += ['-DPNG_ARM_NEON_OPT=2']
endif

libpng = static_library(png_libname,
    png_src,
    dependencies : libpng_deps,
    c_args: c_args,
)

libpng_dep = declare_dependency(
    include_directories : include,
    link_with : libpng,
    dependencies : libpng_deps,
)
# Keep the older dependency name for backwards-compat with old-style
# dependency(..., fallback ['libpng': 'png_dep'])
png_dep = libpng_dep

png_test = executable('pngtest', 'pngtest.c', dependencies : libpng_dep)
test('pngtest', png_test, args : files('pngtest.png'))
