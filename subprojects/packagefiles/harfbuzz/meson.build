project('harfbuzz', 'c', 'cpp',
  meson_version: '>= 0.55.0',
  version: '10.2.0',
  default_options: [
    'cpp_eh=none',          # Just to support msvc, we are passing -fno-exceptions also anyway
    # 'cpp_rtti=false',     # Do NOT enable, wraps inherit it and ICU needs RTTI
    'cpp_std=c++11',
    'wrap_mode=nofallback', # Use --wrap-mode=default to revert, https://github.com/harfbuzz/harfbuzz/pull/2548
  ],
)

chafa_min_version = '>= 1.6.0'
icu_min_version = '>= 49.0'
graphite2_min_version = '>= 1.2.0'

freetype_min_version_actual = '>= 2.4.2'
freetype_min_version = '>= 12.0.6'    # Corresponds to `freetype_min_version_actual`

hb_version_arr = meson.project_version().split('.')
hb_version_major = hb_version_arr[0].to_int()
hb_version_minor = hb_version_arr[1].to_int()
hb_version_micro = hb_version_arr[2].to_int()

# libtool versioning
hb_version_int = 60000 + hb_version_major*100 + hb_version_minor*10 + hb_version_micro
hb_libtool_version_info = '@0@:0:@0@'.format(hb_version_int)

pkgmod = import('pkgconfig')
cpp = meson.get_compiler('cpp')
null_dep = dependency('', required: false)

# Only perform these checks if cpp_std is c++11 as setting -std directly
# produces a warning from meson.
if get_option('cpp_std') == 'c++11'
  # Enforce C++14 requirement for MSVC STL
  if cpp.get_id() == 'clang' and cpp.get_define('_MSC_FULL_VER') != ''
    add_project_arguments('-std=c++14', language: 'cpp')
  elif cpp.get_id() == 'clang-cl'
    # Clang-cl produces a warning when using -std=c++14, but not when using /std:c++14
    add_project_arguments('/std:c++14', language : 'cpp')
  endif
endif

if cpp.get_argument_syntax() == 'msvc'
  # Ignore several spurious warnings for things HarfBuzz does very commonly.
  # If a warning is completely useless and spammy, use '/wdXXXX' to suppress it
  # If a warning is harmless but hard to fix, use '/woXXXX' so it's shown once
  # NOTE: Only add warnings here if you are sure they're spurious
  msvc_args = [
    '/wd4244', # lossy type conversion (e.g. double -> int)
    '/bigobj', # hb-subset.cc -- compile error C1128: number of sections exceeded object file format limit
    cpp.get_supported_arguments(['/utf-8']), # set the input encoding to utf-8
  ]
  add_project_arguments(msvc_args, language: ['c', 'cpp'])
  # Disable SAFESEH with MSVC for libs that use external deps that are built with MinGW
  # noseh_link_args = ['/SAFESEH:NO']
endif

add_project_link_arguments(cpp.get_supported_link_arguments([
  '-Bsymbolic-functions'
]), language: 'c')

add_project_arguments(cpp.get_supported_arguments([
  '-fno-exceptions',
  '-fno-rtti',
  '-fno-threadsafe-statics',
  '-fvisibility-inlines-hidden',
]), language: 'cpp')

if host_machine.cpu_family() == 'arm' and cpp.alignment('struct { char c; }') != 1
  if cpp.has_argument('-mstructure-size-boundary=8')
    add_project_arguments('-mstructure-size-boundary=8', language: 'cpp')
  endif
endif

if host_machine.system() == 'windows'
  add_project_arguments(cpp.get_supported_arguments([
    '-Wa,-mbig-obj'
  ]), language : 'cpp')
endif

check_headers = [
  ['unistd.h'],
  ['sys/mman.h'],
  ['stdbool.h'],
  ['xlocale.h'],
]

check_funcs = [
  ['atexit', {'prefix': '#include <stdlib.h>'}],
  ['mprotect', {'prefix': '#include <sys/mman.h>'}],
  ['sysconf', {'prefix': '#include <unistd.h>'}],
  ['getpagesize', {'prefix': '#include <unistd.h>'}],
  ['mmap', {'prefix': '#include <sys/mman.h>'}],
  ['isatty', {'prefix': '#include <unistd.h>'}],
  ['uselocale', {'prefix': '#include <locale.h>'}],
  ['newlocale', {'prefix': '#include <locale.h>'}],
  ['sincosf', {'prefix': '#define _GNU_SOURCE\n#include <math.h>'}],
]

m_dep = cpp.find_library('m', required: false)

if meson.version().version_compare('>=0.60.0')
  # pkg-config: freetype2, cmake: Freetype
  freetype_dep = dependency('freetype2', 'Freetype',
                            required: get_option('freetype'),
                            default_options: ['harfbuzz=disabled'],
                            allow_fallback: true)
else
  # painful hack to handle multiple dependencies but also respect options
  freetype_opt = get_option('freetype')
  # we want to handle enabled manually after fallbacks, but also handle disabled normally
  if freetype_opt.enabled()
    freetype_opt = false
  endif
  # try pkg-config name
  freetype_dep = dependency('freetype2', method: 'pkg-config', required: freetype_opt)
  # when disabled, leave it not-found
  if not freetype_dep.found() and not get_option('freetype').disabled()
    # Try cmake name
    freetype_dep = dependency('Freetype', method: 'cmake', required: false)
    # Subproject fallback, `allow_fallback: true` means the fallback will be
    # tried even if the freetype option is set to `auto`.
    if not freetype_dep.found()
      freetype_dep = dependency('freetype2',
                                method: 'pkg-config',
                                required: get_option('freetype'),
                                default_options: ['harfbuzz=disabled'],
                                allow_fallback: true)
    endif
  endif
endif

graphite2_dep = dependency('graphite2', version: graphite2_min_version, required: get_option('graphite2'))
graphite_dep = dependency('graphite2', version: graphite2_min_version, required: get_option('graphite'))
wasm_dep = cpp.find_library('iwasm', required: get_option('wasm'))
# How to check whether iwasm was built, and hence requires, LLVM?
#llvm_dep = cpp.find_library('LLVM-15', required: get_option('wasm'))

# pkg-config: icu-uc, cmake: ICU but with components
icu_dep = dependency('icu-uc', 'ICU',
                          version: icu_min_version,
                          components: 'uc',
                          required: get_option('icu'),
                          allow_fallback: true)

if icu_dep.found() and icu_dep.type_name() == 'pkgconfig'
  icu_defs = icu_dep.get_variable(pkgconfig: 'DEFS', default_value: '').split()
  if icu_defs.length() > 0
    add_project_arguments(icu_defs, language: ['c', 'cpp'])
  endif
endif

cairo_dep = null_dep
cairo_ft_dep = null_dep

chafa_dep = dependency('chafa', version: chafa_min_version, required: get_option('chafa'))

conf = configuration_data()
incconfig = include_directories('.')

add_project_arguments('-DHAVE_CONFIG_H', language: ['c', 'cpp'])

warn_cflags = [
  '-Wno-non-virtual-dtor',
]

cpp_args = cpp.get_supported_arguments(warn_cflags)

if wasm_dep.found()
  conf.set('HAVE_WASM', 1)
  conf.set('HB_WASM_MODULE_DIR', '"'+get_option('prefix')+'/'+get_option('libdir')+'/harfbuzz/wasm"')
endif

if graphite2_dep.found()
  conf.set('HAVE_GRAPHITE2', 1)
endif

if icu_dep.found()
  conf.set('HAVE_ICU', 1)
endif

if get_option('icu_builtin')
  conf.set('HAVE_ICU_BUILTIN', 1)
endif

if get_option('experimental_api')
  conf.set('HB_EXPERIMENTAL_API', 1)
endif

if freetype_dep.found()
  conf.set('HAVE_FREETYPE', 1)
  check_freetype_funcs = [
    ['FT_Get_Var_Blend_Coordinates', {'deps': freetype_dep}],
    ['FT_Set_Var_Blend_Coordinates', {'deps': freetype_dep}],
    ['FT_Done_MM_Var', {'deps': freetype_dep}],
    ['FT_Get_Transform', {'deps': freetype_dep}],
  ]

  if freetype_dep.type_name() == 'internal'
    foreach func: check_freetype_funcs
      name = func[0]
      conf.set('HAVE_@0@'.format(name.to_upper()), 1)
    endforeach
  else
    check_funcs += check_freetype_funcs
  endif
endif

gdi_uniscribe_deps = []
# GDI (Uniscribe) (Windows)
if host_machine.system() == 'windows' and not get_option('gdi').disabled()
  if (get_option('directwrite').enabled() and
      not (cpp.has_header('usp10.h') and cpp.has_header('windows.h')))
    error('GDI/Uniscribe was enabled explicitly, but required headers are missing.')
  endif

  gdi_deps_found = true
  foreach usplib : ['usp10', 'gdi32', 'rpcrt4']
    dep = cpp.find_library(usplib, required: get_option('gdi'))
    gdi_deps_found = gdi_deps_found and dep.found()
    gdi_uniscribe_deps += dep
  endforeach

  if gdi_deps_found
    conf.set('HAVE_UNISCRIBE', 1)
    conf.set('HAVE_GDI', 1)
  endif
endif

# DirectWrite (Windows)
if host_machine.system() == 'windows' and not get_option('directwrite').disabled()
  if get_option('directwrite').enabled() and not cpp.has_header('dwrite_1.h')
    error('DirectWrite was enabled explicitly, but required header is missing.')
  endif

  conf.set('HAVE_DIRECTWRITE', 1)
endif

# CoreText (macOS)
coretext_deps = []
if host_machine.system() == 'darwin' and not get_option('coretext').disabled()
  app_services_dep = dependency('appleframeworks', modules: ['ApplicationServices'], required: false)
  if cpp.has_type('CTFontRef', prefix: '#include <ApplicationServices/ApplicationServices.h>', dependencies: app_services_dep)
    coretext_deps += [app_services_dep]
    conf.set('HAVE_CORETEXT', 1)
  # On iOS CoreText and CoreGraphics are stand-alone frameworks
  # Check for a different symbol to avoid getting cached result
  else
    coretext_dep = dependency('appleframeworks', modules: ['CoreText'], required: false)
    coregraphics_dep = dependency('appleframeworks', modules: ['CoreGraphics'], required: false)
    corefoundation_dep = dependency('appleframeworks', modules: ['CoreFoundation'], required: false)
    if cpp.has_type('CTRunRef', prefix: '#include <CoreText/CoreText.h>', dependencies: [coretext_dep, coregraphics_dep, corefoundation_dep])
      coretext_deps += [coretext_dep, coregraphics_dep, corefoundation_dep]
      conf.set('HAVE_CORETEXT', 1)
    elif get_option('coretext').enabled()
      error('CoreText was enabled explicitly, but required headers or frameworks are missing.')
    endif
  endif
endif

# threads
thread_dep = null_dep
if host_machine.system() != 'windows'
  thread_dep = dependency('threads', required: false)

  if thread_dep.found()
    conf.set('HAVE_PTHREAD', 1)
  endif
endif

conf.set_quoted('PACKAGE_NAME', 'HarfBuzz')
conf.set_quoted('PACKAGE_VERSION', meson.project_version())

foreach check : check_headers
  name = check[0]

  if cpp.has_header(name)
    conf.set('HAVE_@0@'.format(name.to_upper().underscorify()), 1)
  endif
endforeach

harfbuzz_extra_deps = []
foreach check : check_funcs
  name = check[0]
  opts = check.get(1, {})
  link_withs = opts.get('link_with', [])
  check_deps = opts.get('deps', [])
  check_prefix = opts.get('prefix', '')
  extra_deps = []
  found = true

  # First try without linking
  found = cpp.has_function(name, prefix: check_prefix, dependencies: check_deps)

  if not found and link_withs.length() > 0
    found = true

    foreach link_with : link_withs
      dep = cpp.find_library(link_with, required: false)
      if dep.found()
        extra_deps += dep
      else
        found = false
      endif
    endforeach

    if found
      found = cpp.has_function(name, prefix: check_prefix, dependencies: check_deps + extra_deps)
    endif
  endif

  if found
    harfbuzz_extra_deps += extra_deps
    conf.set('HAVE_@0@'.format(name.to_upper()), 1)
  endif
endforeach

subdir('src')

configure_file(output: 'config.h', configuration: conf)

build_summary = {
  'Unicode callbacks (you want at least one)':
    {'Builtin': true,
     'ICU': conf.get('HAVE_ICU', 0) == 1,
    },
  'Font callbacks (the more the merrier)':
    {'FreeType': conf.get('HAVE_FREETYPE', 0) == 1,
    },
  'Dependencies used for command-line utilities':
    {'Cairo': conf.get('HAVE_CAIRO', 0) == 1,
     'Chafa': conf.get('HAVE_CHAFA', 0) == 1,
    },
  'Additional shapers':
    {'Graphite2': conf.get('HAVE_GRAPHITE2', 0) == 1,
     'WebAssembly (experimental)': conf.get('HAVE_WASM', 0) == 1,
    },
  'Platform shapers (not normally needed)':
    {'CoreText': conf.get('HAVE_CORETEXT', 0) == 1,
     'DirectWrite (experimental)': conf.get('HAVE_DIRECTWRITE', 0) == 1,
     'GDI/Uniscribe': (conf.get('HAVE_GDI', 0) == 1) and (conf.get('HAVE_UNISCRIBE', 0) == 1),
    },
}
foreach section_title, section : build_summary
  summary(section, bool_yn: true, section: section_title)
endforeach

