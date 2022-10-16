# Modifications of HarfBuzz meson.build for Arcollect
project('harfbuzz', 'c', 'cpp',
  version: '5.3.0',
)

hb_version_arr = meson.project_version().split('.')
hb_version_major = hb_version_arr[0].to_int()
hb_version_minor = hb_version_arr[1].to_int()
hb_version_micro = hb_version_arr[2].to_int()

# libtool versioning
hb_version_int = hb_version_major*10000 + hb_version_minor*100 + hb_version_micro
hb_libtool_version_info = '@0@:0:@0@'.format(hb_version_int)

pkgmod = import('pkgconfig')
cpp = meson.get_compiler('cpp')
null_dep = dependency('', required: false)

if cpp.get_argument_syntax() == 'msvc'
  # Ignore several spurious warnings for things HarfBuzz does very commonly.
  # If a warning is completely useless and spammy, use '/wdXXXX' to suppress it
  # If a warning is harmless but hard to fix, use '/woXXXX' so it's shown once
  # NOTE: Only add warnings here if you are sure they're spurious
  msvc_args = [
    '/wd4018', # implicit signed/unsigned conversion
    '/wd4146', # unary minus on unsigned (beware INT_MIN)
    '/wd4244', # lossy type conversion (e.g. double -> int)
    '/wd4305', # truncating type conversion (e.g. double -> float)
    cpp.get_supported_arguments(['/utf-8']), # set the input encoding to utf-8
  ]
  add_project_arguments(msvc_args, language: ['c', 'cpp'])
  # Disable SAFESEH with MSVC for libs that use external deps that are built with MinGW
  # noseh_link_args = ['/SAFESEH:NO']
  # disable exception handling
  add_project_arguments(['/EHs-', '/EHc-'], language: 'cpp')
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
  ['atexit'],
  ['mprotect'],
  ['sysconf'],
  ['getpagesize'],
  ['mmap'],
  ['isatty'],
  ['uselocale'],
  ['newlocale'],
]

m_dep = cpp.find_library('m', required: false)

freetype_dep = dependency('freetype2', required: get_option('freetype'))

glib_dep = disabler()
gobject_dep =  disabler()
graphite2_dep = dependency('graphite2', required: get_option('graphite2'))

# Try pkgconfig name
icu_dep = dependency('icu-uc', required: false)
if not icu_dep.found()
  # Try cmake name
  icu_dep = dependency('ICU',
                       required: false,
                       components: 'uc',
                       method: 'cmake')
endif
if not icu_dep.found()
  # Subproject fallback if icu option is enabled
  icu_dep = dependency('icu-uc', required: get_option('icu'))
endif

if icu_dep.found() and icu_dep.type_name() == 'pkgconfig'
  icu_defs = icu_dep.get_variable(pkgconfig: 'DEFS', default_value: '').split()
  if icu_defs.length() > 0
    add_project_arguments(icu_defs, language: ['c', 'cpp'])
  endif
endif

conf = configuration_data()
incconfig = include_directories('.')

add_project_arguments('-DHAVE_CONFIG_H', language: ['c', 'cpp'])

warn_cflags = [
  '-Wno-non-virtual-dtor',
]

cpp_args = cpp.get_supported_arguments(warn_cflags)

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
  extra_deps = []
  found = true

  # First try without linking
  found = cpp.has_function(name, dependencies: check_deps)

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
      found = cpp.has_function(name, dependencies: check_deps + extra_deps)
    endif
  endif

  if found
    harfbuzz_extra_deps += extra_deps
    conf.set('HAVE_@0@'.format(name.to_upper()), 1)
  endif
endforeach

subdir('src')

# Strip uneeded functionalities
conf.set('HB_DISABLE_DEPRECATED'      ,1) # Deprecation ? No thanks.
conf.set('HB_NO_BUFFER_MESSAGE'       ,1) # Nuke hb_buffer_message_func_t
conf.set('HB_NO_BUFFER_SERIALIZE'     ,1) # Nuke hb_buffer_serialize_glyphs
conf.set('HB_NO_BUFFER_VERIFY'        ,1) # Nuke the unused buffer sanity check
conf.set('HB_NO_CMAP_LEGACY_SUBTABLES',1) # Remove as much legacy as we can
conf.set('HB_NO_DRAW'                 ,1) # We use FreeType2 to render fonts
conf.set('HB_NO_FACE_COLLECT_UNICODES',1) # Nuke unused API
conf.set('HB_NO_FALLBACK_SHAPE'       ,1) # Remove as much legacy as we can
conf.set('HB_NO_MT'                   ,1) # HarfBuzz is always used in the main thread
conf.set('HB_NO_OT_FONT'              ,1) # We always use FreeType2
conf.set('HB_NO_OT_FONT_GLYPH_NAMES'  ,1) # We never query glyph names
conf.set('HB_NO_OT_LAYOUT_BLOCKLIST'  ,1) # Remove as much legacy as we can
conf.set('HB_NO_OT_SHAPE_FALLBACK'    ,1) # Remove as much legacy as we can
conf.set('NDEBUG'                     ,1) # No debug
configure_file(output: 'config.h', configuration: conf)

build_summary = {
  'Unicode callbacks (you want at least one)':
    {'Builtin': true,
     'ICU': conf.get('HAVE_ICU', 0) == 1,
    },
  'Font callbacks (the more the merrier)':
    {'FreeType': conf.get('HAVE_FREETYPE', 0) == 1,
    },
  'Additional shapers':
    {'Graphite2': conf.get('HAVE_GRAPHITE2', 0) == 1,
    },
  'Platform shapers (not normally needed)':
    {'CoreText': conf.get('HAVE_CORETEXT', 0) == 1,
     'DirectWrite': conf.get('HAVE_DIRECTWRITE', 0) == 1,
     'GDI/Uniscribe': (conf.get('HAVE_GDI', 0) == 1) and (conf.get('HAVE_UNISCRIBE', 0) == 1),
    },
  'Other features':
    {'Experimental APIs': conf.get('HB_EXPERIMENTAL_API', 0) == 1,
    },
}
foreach section_title, section : build_summary
  summary(section, bool_yn: true, section: section_title)
endforeach
