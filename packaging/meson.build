packages_files = [
]

packages_conf = configuration_data()
packages_conf.merge_from(config_h)
packages_conf.set('MESON_BUILD_ROOT', meson.project_build_root())
packages_conf.set('MESON_CURRENT_BUILD_DIR', meson.current_build_dir())
packages_conf.set('MESON_SOURCE_ROOT', meson.project_source_root())

make_packaging_for = get_option('make_packaging_for')
# ArchLinux PKGBUILD
# Two favlors are generated:
# 	PKGBUILD usable by anybody
# 	PKGBUILD.local that use local directory as source directory
pkgbuild_conf = configuration_data()
pkgbuild_conf.merge_from(config_h)
pkgbuild_conf.set_quoted('PKGBUILD_SOURCE', 'https://github.com/DevilishSpirits/arcollect/archive/refs/tags/v$pkgver.tar.gz')
pkgbuild_conf.set_quoted('PKGBUILD_MD5SUMS', 'SKIP')
pkgbuild_conf.set('PKGBUILD_BUILDDIR', 'PKGBUILD.build')
pkgbuild_conf.set_quoted('PKGBUILD_SRCDIR', '$srcdir/arcollect-$pkgver')
configure_file(input: 'PKGBUILD.in', output: 'PKGBUILD', configuration: pkgbuild_conf)

pkgbuild_local_conf = configuration_data()
pkgbuild_local_conf.merge_from(config_h)
pkgbuild_local_conf.set('PKGBUILD_SOURCE', '')
pkgbuild_local_conf.set('PKGBUILD_MD5SUMS', '')
pkgbuild_local_conf.set('PKGBUILD_BUILDDIR', 'PKGBUILD.local-build')
pkgbuild_local_conf.set('PKGBUILD_SRCDIR', meson.project_source_root())
pkgbuild_local = configure_file(input: 'PKGBUILD.in', output: 'PKGBUILD.local', configuration: pkgbuild_local_conf)

if make_packaging_for.contains('pacman')
	pkgbuild_output = 'arcollect-'+meson.project_version()+'-1-'+host_machine.cpu_family()+'.pkg.tar.zst'
	custom_target(pkgbuild_output, command: [
		find_program('sh', native: true),'-c',
		'cd "'+meson.current_build_dir()+'" && '+find_program('makepkg', native: true).full_path()+' -p PKGBUILD.local -f -c CFLAGS= CXXFLAGS= LDFLAGS= PKGEXT=".pkg.tar.zst"'
		], output: pkgbuild_output, build_by_default: true)
endif

foreach packages_file: packages_files
	configure_file(input: packages_file+'.in', output: packages_file, configuration: packages_conf)
endforeach
