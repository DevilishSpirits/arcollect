hb_version_h = configure_file(
               command: [find_program('gen-hb-version.py'), meson.project_version(), '@OUTPUT@', '@INPUT@'],
               input: 'hb-version.h.in',
               output: 'hb-version.h',
               install: false)

# Base and default-included sources and headers
hb_base_sources = files(
  'hb-aat-layout-ankr-table.hh',
  'hb-aat-layout-bsln-table.hh',
  'hb-aat-layout-common.hh',
  'hb-aat-layout-feat-table.hh',
  'hb-aat-layout-just-table.hh',
  'hb-aat-layout-kerx-table.hh',
  'hb-aat-layout-morx-table.hh',
  'hb-aat-layout-opbd-table.hh',
  'hb-aat-layout-trak-table.hh',
  'hb-aat-layout.cc',
  'hb-aat-layout.hh',
  'hb-aat-ltag-table.hh',
  'hb-aat-map.cc',
  'hb-aat-map.hh',
  'hb-algs.hh',
  'hb-array.hh',
  'hb-atomic.hh',
  'hb-bimap.hh',
  'hb-bit-page.hh',
  'hb-blob.cc',
  'hb-blob.hh',
  'hb-buffer-serialize.cc',
  'hb-buffer-verify.cc',
  'hb-buffer.cc',
  'hb-buffer.hh',
  'hb-cache.hh',
  'hb-cff-interp-common.hh',
  'hb-cff-interp-cs-common.hh',
  'hb-cff-interp-dict-common.hh',
  'hb-cff1-interp-cs.hh',
  'hb-cff2-interp-cs.hh',
  'hb-common.cc',
  'hb-config.hh',
  'hb-debug.hh',
  'hb-dispatch.hh',
  'hb-draw.cc',
  'hb-draw.hh',
  'hb-paint.cc',
  'hb-paint.hh',
  'hb-paint-extents.cc',
  'hb-paint-extents.hh',
  'hb-face.cc',
  'hb-face.hh',
  'hb-face-builder.cc',
  'hb-fallback-shape.cc',
  'hb-font.cc',
  'hb-font.hh',
  'hb-iter.hh',
  'hb-kern.hh',
  'hb-limits.hh',
  'hb-machinery.hh',
  'hb-map.cc',
  'hb-map.hh',
  'hb-meta.hh',
  'hb-ms-feature-ranges.hh',
  'hb-multimap.hh',
  'hb-mutex.hh',
  'hb-null.hh',
  'hb-number.cc',
  'hb-number.hh',
  'hb-object.hh',
  'hb-open-file.hh',
  'hb-open-type.hh',
  'hb-ot-cff-common.hh',
  'hb-ot-cff1-std-str.hh',
  'hb-ot-cff1-table.cc',
  'hb-ot-cff1-table.hh',
  'hb-ot-cff2-table.cc',
  'hb-ot-cff2-table.hh',
  'hb-ot-cmap-table.hh',
  'hb-ot-color.cc',
  'hb-ot-face-table-list.hh',
  'hb-ot-face.cc',
  'hb-ot-face.hh',
  'hb-ot-font.cc',
  'hb-ot-gasp-table.hh',
  'hb-ot-glyf-table.hh',
  'hb-ot-hdmx-table.hh',
  'hb-ot-head-table.hh',
  'hb-ot-hhea-table.hh',
  'hb-ot-hmtx-table.hh',
  'hb-ot-kern-table.hh',
  'hb-ot-layout-base-table.hh',
  'hb-ot-layout-common.hh',
  'hb-ot-layout-gdef-table.hh',
  'hb-ot-layout-gpos-table.hh',
  'hb-ot-layout-gsub-table.hh',
  'hb-outline.hh',
  'hb-outline.cc',
  'OT/Color/CBDT/CBDT.hh',
  'OT/Color/COLR/COLR.hh',
  'OT/Color/CPAL/CPAL.hh',
  'OT/Color/sbix/sbix.hh',
  'OT/Color/svg/svg.hh',
  'OT/glyf/glyf.hh',
  'OT/glyf/glyf-helpers.hh',
  'OT/glyf/loca.hh',
  'OT/glyf/path-builder.hh',
  'OT/glyf/Glyph.hh',
  'OT/glyf/GlyphHeader.hh',
  'OT/glyf/SimpleGlyph.hh',
  'OT/glyf/CompositeGlyph.hh',
  'OT/glyf/SubsetGlyph.hh',
  'OT/Layout/types.hh',
  'OT/Layout/Common/Coverage.hh',
  'OT/Layout/Common/CoverageFormat1.hh',
  'OT/Layout/Common/CoverageFormat2.hh',
  'OT/Layout/Common/RangeRecord.hh',
  'OT/Layout/GDEF/GDEF.hh',
  'OT/Layout/GPOS/AnchorFormat1.hh',
  'OT/Layout/GPOS/AnchorFormat2.hh',
  'OT/Layout/GPOS/AnchorFormat3.hh',
  'OT/Layout/GPOS/Anchor.hh',
  'OT/Layout/GPOS/AnchorMatrix.hh',
  'OT/Layout/GPOS/ChainContextPos.hh',
  'OT/Layout/GPOS/Common.hh',
  'OT/Layout/GPOS/ContextPos.hh',
  'OT/Layout/GPOS/CursivePosFormat1.hh',
  'OT/Layout/GPOS/CursivePos.hh',
  'OT/Layout/GPOS/ExtensionPos.hh',
  'OT/Layout/GPOS/GPOS.hh',
  'OT/Layout/GPOS/LigatureArray.hh',
  'OT/Layout/GPOS/MarkArray.hh',
  'OT/Layout/GPOS/MarkBasePosFormat1.hh',
  'OT/Layout/GPOS/MarkBasePos.hh',
  'OT/Layout/GPOS/MarkLigPosFormat1.hh',
  'OT/Layout/GPOS/MarkLigPos.hh',
  'OT/Layout/GPOS/MarkMarkPosFormat1.hh',
  'OT/Layout/GPOS/MarkMarkPos.hh',
  'OT/Layout/GPOS/MarkRecord.hh',
  'OT/Layout/GPOS/PairPosFormat1.hh',
  'OT/Layout/GPOS/PairPosFormat2.hh',
  'OT/Layout/GPOS/PairPos.hh',
  'OT/Layout/GPOS/PairSet.hh',
  'OT/Layout/GPOS/PairValueRecord.hh',
  'OT/Layout/GPOS/PosLookup.hh',
  'OT/Layout/GPOS/PosLookupSubTable.hh',
  'OT/Layout/GPOS/SinglePosFormat1.hh',
  'OT/Layout/GPOS/SinglePosFormat2.hh',
  'OT/Layout/GPOS/SinglePos.hh',
  'OT/Layout/GPOS/ValueFormat.hh',
  'OT/Layout/GSUB/AlternateSet.hh',
  'OT/Layout/GSUB/AlternateSubstFormat1.hh',
  'OT/Layout/GSUB/AlternateSubst.hh',
  'OT/Layout/GSUB/ChainContextSubst.hh',
  'OT/Layout/GSUB/Common.hh',
  'OT/Layout/GSUB/ContextSubst.hh',
  'OT/Layout/GSUB/ExtensionSubst.hh',
  'OT/Layout/GSUB/GSUB.hh',
  'OT/Layout/GSUB/Ligature.hh',
  'OT/Layout/GSUB/LigatureSet.hh',
  'OT/Layout/GSUB/LigatureSubstFormat1.hh',
  'OT/Layout/GSUB/LigatureSubst.hh',
  'OT/Layout/GSUB/MultipleSubstFormat1.hh',
  'OT/Layout/GSUB/MultipleSubst.hh',
  'OT/Layout/GSUB/ReverseChainSingleSubstFormat1.hh',
  'OT/Layout/GSUB/ReverseChainSingleSubst.hh',
  'OT/Layout/GSUB/Sequence.hh',
  'OT/Layout/GSUB/SingleSubstFormat1.hh',
  'OT/Layout/GSUB/SingleSubstFormat2.hh',
  'OT/Layout/GSUB/SingleSubst.hh',
  'OT/Layout/GSUB/SubstLookup.hh',
  'OT/Layout/GSUB/SubstLookupSubTable.hh',
  'OT/name/name.hh',
  'hb-ot-layout-gsubgpos.hh',
  'hb-ot-layout-jstf-table.hh',
  'hb-ot-layout.cc',
  'hb-ot-layout.hh',
  'hb-ot-map.cc',
  'hb-ot-map.hh',
  'hb-ot-math-table.hh',
  'hb-ot-math.cc',
  'hb-ot-maxp-table.hh',
  'hb-ot-meta-table.hh',
  'hb-ot-meta.cc',
  'hb-ot-metrics.cc',
  'hb-ot-metrics.hh',
  'hb-ot-name-language-static.hh',
  'hb-ot-name-language.hh',
  'hb-ot-name-table.hh',
  'hb-ot-name.cc',
  'hb-ot-os2-table.hh',
  'hb-ot-os2-unicode-ranges.hh',
  'hb-ot-post-macroman.hh',
  'hb-ot-post-table.hh',
  'hb-ot-shaper-arabic-fallback.hh',
  'hb-ot-shaper-arabic-joining-list.hh',
  'hb-ot-shaper-arabic-pua.hh',
  'hb-ot-shaper-arabic-table.hh',
  'hb-ot-shaper-arabic-win1256.hh',
  'hb-ot-shaper-arabic.cc',
  'hb-ot-shaper-arabic.hh',
  'hb-ot-shaper-default.cc',
  'hb-ot-shaper-hangul.cc',
  'hb-ot-shaper-hebrew.cc',
  'hb-ot-shaper-indic-table.cc',
  'hb-ot-shaper-indic.cc',
  'hb-ot-shaper-indic.hh',
  'hb-ot-shaper-khmer.cc',
  'hb-ot-shaper-myanmar.cc',
  'hb-ot-shaper-syllabic.cc',
  'hb-ot-shaper-syllabic.hh',
  'hb-ot-shaper-thai.cc',
  'hb-ot-shaper-use-table.hh',
  'hb-ot-shaper-use.cc',
  'hb-ot-shaper-vowel-constraints.cc',
  'hb-ot-shaper-vowel-constraints.hh',
  'hb-ot-shaper.hh',
  'hb-ot-shape-fallback.cc',
  'hb-ot-shape-fallback.hh',
  'hb-ot-shape-normalize.cc',
  'hb-ot-shape-normalize.hh',
  'hb-ot-shape.cc',
  'hb-ot-shape.hh',
  'hb-ot-stat-table.hh',
  'hb-ot-tag-table.hh',
  'hb-ot-tag.cc',
  'hb-ot-var-avar-table.hh',
  'hb-ot-var-common.hh',
  'hb-ot-var-cvar-table.hh',
  'hb-ot-var-fvar-table.hh',
  'hb-ot-var-gvar-table.hh',
  'hb-ot-var-hvar-table.hh',
  'hb-ot-var-mvar-table.hh',
  'hb-ot-var.cc',
  'hb-ot-vorg-table.hh',
  'hb-pool.hh',
  'hb-sanitize.hh',
  'hb-serialize.hh',
  'hb-set-digest.hh',
  'hb-set.cc',
  'hb-set.hh',
  'hb-shape-plan.cc',
  'hb-shape-plan.hh',
  'hb-shape.cc',
  'hb-shaper-impl.hh',
  'hb-shaper-list.hh',
  'hb-shaper.cc',
  'hb-shaper.hh',
  'hb-static.cc',
  'hb-string-array.hh',
  'hb-style.cc',
  'hb-ucd-table.hh',
  'hb-ucd.cc',
  'hb-unicode-emoji-table.hh',
  'hb-unicode.cc',
  'hb-unicode.hh',
  'hb-utf.hh',
  'hb-vector.hh',
  'hb.hh',
)

hb_base_ragel_generated_sources = files(
  'hb-buffer-deserialize-json.hh',
  'hb-buffer-deserialize-text-glyphs.hh',
  'hb-buffer-deserialize-text-unicode.hh',
  'hb-number-parser.hh',
  'hb-ot-shaper-indic-machine.hh',
  'hb-ot-shaper-khmer-machine.hh',
  'hb-ot-shaper-myanmar-machine.hh',
  'hb-ot-shaper-use-machine.hh',
)
hb_base_ragel_sources = [
  'hb-buffer-deserialize-json.rl',
  'hb-buffer-deserialize-text-glyphs.rl',
  'hb-buffer-deserialize-text-unicode.rl',
  'hb-number-parser.rl',
  'hb-ot-shaper-indic-machine.rl',
  'hb-ot-shaper-khmer-machine.rl',
  'hb-ot-shaper-myanmar-machine.rl',
  'hb-ot-shaper-use-machine.rl',
]

hb_base_headers = files(
  'hb-aat-layout.h',
  'hb-aat.h',
  'hb-blob.h',
  'hb-buffer.h',
  'hb-common.h',
  'hb-cplusplus.hh',
  'hb-deprecated.h',
  'hb-draw.h',
  'hb-paint.h',
  'hb-face.h',
  'hb-font.h',
  'hb-map.h',
  'hb-ot-color.h',
  'hb-ot-deprecated.h',
  'hb-ot-font.h',
  'hb-ot-layout.h',
  'hb-ot-math.h',
  'hb-ot-meta.h',
  'hb-ot-metrics.h',
  'hb-ot-name.h',
  'hb-ot-shape.h',
  'hb-ot-var.h',
  'hb-ot.h',
  'hb-set.h',
  'hb-shape-plan.h',
  'hb-shape.h',
  'hb-style.h',
  'hb-unicode.h',
  'hb.h',
)
hb_base_headers += hb_version_h

# Optional Sources and Headers with external deps

hb_ft_sources = files('hb-ft.cc', 'hb-ft-colr.hh')
hb_ft_headers = files('hb-ft.h')

hb_graphite2_sources = files('hb-graphite2.cc')
hb_graphite2_headers = files('hb-graphite2.h')

# System-dependent sources and headers

hb_coretext_sources = files('hb-coretext.cc')
hb_coretext_headers = files('hb-coretext.h')

hb_directwrite_sources = files('hb-directwrite.cc')
hb_directwrite_headers = files('hb-directwrite.h')

hb_gdi_sources = files('hb-gdi.cc')
hb_gdi_headers = files('hb-gdi.h')

hb_uniscribe_sources = files('hb-uniscribe.cc')
hb_uniscribe_headers = files('hb-uniscribe.h')

# Sources for libharfbuzz-gobject and libharfbuzz-icu
hb_icu_sources = files('hb-icu.cc')
hb_icu_headers = files('hb-icu.h')

ragel = find_program('ragel', version: '6.10', required: false)
has_ragel = ragel.found()
if not has_ragel and get_option('ragel_subproject')
    ragel = subproject('ragel').get_variable('ragel')
    has_ragel = true
endif
if not has_ragel
  if not meson.is_subproject()
    warning('You have to install ragel if you are going to develop HarfBuzz itself')
  endif
else
  ragel_helper = find_program('gen-ragel-artifacts.py')
  foreach rl : hb_base_ragel_sources
    hh = rl.split('.')[0] + '.hh'
    custom_target('@0@'.format(hh),
      build_by_default: false,
      input: rl,
      output: hh,
      command: [ragel_helper, ragel, '@OUTPUT@', meson.current_source_dir(), '@INPUT@'],
    )
  endforeach
endif

custom_target('harfbuzz.cc',
  build_by_default: false,
  output: 'harfbuzz.cc',
  input: hb_base_sources + hb_ft_sources +
         hb_graphite2_sources + hb_uniscribe_sources + hb_gdi_sources +
         hb_directwrite_sources + hb_coretext_sources,
  command: [find_program('gen-harfbuzzcc.py'),
            '@OUTPUT@', meson.current_source_dir(), '@INPUT@'],
)

incsrc = include_directories('.')

hb_sources = hb_base_sources + hb_base_ragel_generated_sources
hb_headers = hb_base_headers

harfbuzz_deps = [thread_dep, m_dep] + harfbuzz_extra_deps

libharfbuzz_link_language = 'c'

if conf.get('HAVE_FREETYPE', 0) == 1
  hb_sources += hb_ft_sources
  hb_headers += hb_ft_headers
  harfbuzz_deps += [freetype_dep]
endif

if conf.get('HAVE_GDI', 0) == 1
  hb_sources += hb_gdi_sources
  hb_headers += hb_gdi_headers
  harfbuzz_deps += gdi_uniscribe_deps
endif

if conf.get('HAVE_GRAPHITE2', 0) == 1
  hb_sources += hb_graphite2_sources
  hb_headers += hb_graphite2_headers
  harfbuzz_deps += [graphite2_dep]
endif

if conf.get('HAVE_UNISCRIBE', 0) == 1
  hb_sources += hb_uniscribe_sources
  hb_headers += hb_uniscribe_headers
endif

if conf.get('HAVE_DIRECTWRITE', 0) == 1
  hb_sources += hb_directwrite_sources
  hb_headers += hb_directwrite_headers
  # hb-directwrite needs a C++ linker
  libharfbuzz_link_language = 'cpp'
endif

if conf.get('HAVE_CORETEXT', 0) == 1
  hb_sources += hb_coretext_sources
  hb_headers += hb_coretext_headers
  harfbuzz_deps += coretext_deps
endif

have_icu = conf.get('HAVE_ICU', 0) == 1
have_icu_builtin = conf.get('HAVE_ICU_BUILTIN', 0) == 1
if have_icu and have_icu_builtin
  hb_sources += hb_icu_sources
  hb_headers += hb_icu_headers
  harfbuzz_deps += [icu_dep]
endif

# harfbuzz
version = '0.@0@.0'.format(hb_version_int)

extra_hb_cpp_args = []
if cpp.get_argument_syntax() == 'msvc'
  if get_option('default_library') != 'static'
    extra_hb_cpp_args += '-DHB_DLL_EXPORT'
  endif
  hb_so_version = ''
else
  hb_so_version = '0'
endif

if get_option('fuzzer_ldflags') != ''
  extra_hb_cpp_args += ['-DHB_CUSTOM_MALLOC']
  hb_sources += 'failing-alloc.c'
  hb_subset_sources += 'failing-alloc.c'
  hb_icu_sources += 'failing-alloc.c'
  hb_gobject_sources += 'failing-alloc.c'
endif

darwin_versions = [hb_version_int, '@0@.0.0'.format(hb_version_int)]

libharfbuzz = library('harfbuzz', hb_sources,
  include_directories: incconfig,
  dependencies: harfbuzz_deps,
  cpp_args: cpp_args + extra_hb_cpp_args,
  soversion: hb_so_version,
  version: version,
  install: false,
  darwin_versions: darwin_versions,
  link_language: libharfbuzz_link_language,
)

libharfbuzz_dep = declare_dependency(
  link_with: libharfbuzz,
  include_directories: incsrc,
  dependencies: harfbuzz_deps)

libharfbuzz_icu_dep = null_dep
if have_icu and not have_icu_builtin
  libharfbuzz_icu = library('harfbuzz-icu', [hb_icu_sources, hb_icu_headers],
    include_directories: incconfig,
    dependencies: icu_dep,
    link_with: [libharfbuzz],
    cpp_args: cpp_args + extra_hb_cpp_args,
    soversion: hb_so_version,
    version: version,
    install: false,
    darwin_versions: darwin_versions,
    # ICU links to stdc++ anyway so the default linker is good
    # link_language: 'c',
  )

  libharfbuzz_icu_dep = declare_dependency(
    link_with: libharfbuzz_icu,
    include_directories: incsrc,
    dependencies: icu_dep)
  meson.override_dependency('harfbuzz-icu', libharfbuzz_icu_dep)
endif
