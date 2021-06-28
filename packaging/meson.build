packages_files = [
]

packages_conf = configuration_data()
packages_conf.merge_from(config_h)
packages_conf.set('MESON_BUILD_ROOT', meson.project_build_root())
packages_conf.set('MESON_CURRENT_BUILD_DIR', meson.current_build_dir())
packages_conf.set('MESON_SOURCE_ROOT', meson.project_source_root())

sh_prog = find_program('sh', required: false, native: true)

# DESTDIR target
# This special target run `meson compile` to a DESTDIR
destdir_depends = []
destdir_depend_files = []
if enable_native_progs
	destdir_depends += [webext_adder_exe,arcollect_exe]
endif
if enable_webextension
	destdir_depend_files += [webext_appmanifest_firefox]
endif
destdir_target = custom_target('destdir',
	command: [find_program('meson', native: true), 'install', '-C', '@BUILD_ROOT@'],
	env: ['DESTDIR='+meson.current_build_dir() / 'DESTDIR'],
	output: 'DESTDIR',
	depends: destdir_depends,
	depend_files: destdir_depend_files,
	build_by_default: false,
)

# destdir_md5sum
# This target run md5sum on DESTDIR (destdir_target)
md5sum_target = custom_target('destdir_md5sum',
	command: [sh_prog,'-c','cd \''+destdir_target.full_path()+'\' && find usr -type f | exec xargs md5sum'],
	output: 'md5sums',
	capture: true,
	depends: destdir_target,
	build_by_default: false,
)

# Check what have been statically linked
if enable_native_progs
	dep_curl_static = dep_curl.type_name() == 'internal'
	dep_sdl2_static = dep_sdl2.type_name() == 'internal'
	dep_sdl2_image_static = dep_sdl2_image.type_name() == 'internal'
	dep_sdl2_ttf_static = dep_sdl2_ttf.type_name() == 'internal'
	dep_sqlite3_static = dep_sqlite3.type_name() == 'internal'
	dep_inih_static = dep_inih.type_name() == 'internal'
	summary({
		'curl': not dep_curl_static,
		'sdl2': not dep_sdl2_static,
		'sdl2_image': not dep_sdl2_image_static,
		'sdl2_ttf': not dep_sdl2_ttf_static,
		'SQLite3': not dep_sqlite3_static,
		'INIReader': not dep_inih_static,
		
		'freetype2': dep_freetype2.type_name() != 'internal',
		'libjpeg': dep_libjpeg.type_name() != 'internal',
		'libpng': dep_libpng.type_name() != 'internal',
		'libtiff-4': dep_libtiff.type_name() != 'internal',
	}, section: 'Using system dependency (NO if embeded)', bool_yn: true)
endif

# ArchLinux PKGBUILD
# Two favlors are generated:
# 	PKGBUILD usable by anybody
# 	PKGBUILD.local that use local directory as source directory
pkgbuild_depends = []
pkgbuild_depends_list = {
	# 'dependency_name': 'ArchLinux package name'
	'curl'             : 'curl',
	#'sdl2' has special handling
	'sdl2_image'       : 'sdl2_image',
	'sdl2_ttf'         : 'sdl2_ttf',
	'sqlite3'          : 'sqlite',
	'inih'             : 'libINIReader.so',
}
if enable_native_progs
	foreach dep_name, package_name: pkgbuild_depends_list
		if not get_variable('dep_'+dep_name+'_static')
			pkgbuild_depends += '\''+package_name+config_h.get('ARCOLLECT_VERREQ_'+dep_name,'')+'\''
		endif
	endforeach
	# namcap report warning upon redundant dependencies and sdl2 might be one
	if (not dep_sdl2_static) and dep_sdl2_static and dep_sdl2_ttf_static and (not config_h.has('ARCOLLECT_VERREQ_sdl2'))
		pkgbuild_depends += '\'sdl2'+config_h.get('ARCOLLECT_VERREQ_sdl2','')+'\''
	endif
endif

pkgbuild_conf = configuration_data()
pkgbuild_conf.merge_from(config_h)
pkgbuild_conf.set_quoted('PKGBUILD_SOURCE', 'https://github.com/DevilishSpirits/arcollect/archive/refs/tags/v$pkgver.tar.gz')
pkgbuild_conf.set('PKGBUILD_DEPENDS', ' '.join(pkgbuild_depends))
pkgbuild_conf.set_quoted('PKGBUILD_MD5SUMS', 'SKIP')
pkgbuild_conf.set('PKGBUILD_BUILDDIR', 'PKGBUILD.build')
pkgbuild_conf.set_quoted('PKGBUILD_SRCDIR', '$srcdir/arcollect-$pkgver')
configure_file(input: 'PKGBUILD.in', output: 'PKGBUILD', configuration: pkgbuild_conf)

pkgbuild_local_conf = configuration_data()
pkgbuild_local_conf.merge_from(config_h)
pkgbuild_local_conf.set('PKGBUILD_SOURCE', '')
pkgbuild_local_conf.set('PKGBUILD_DEPENDS', ' '.join(pkgbuild_depends))
pkgbuild_local_conf.set('PKGBUILD_MD5SUMS', '')
pkgbuild_local_conf.set('PKGBUILD_BUILDDIR', 'PKGBUILD.local-build')
pkgbuild_local_conf.set('PKGBUILD_SRCDIR', meson.project_source_root())
pkgbuild_local = configure_file(input: 'PKGBUILD.in', output: 'PKGBUILD.local', configuration: pkgbuild_local_conf)

foreach packages_file: packages_files
	configure_file(input: packages_file+'.in', output: packages_file, configuration: packages_conf)
endforeach

# Debian
# FIXME Generated a binary package by hand is really dirty !
# The Debian package generation consist of the generation a binary package
# manually.
dpkg_architecture_prog = find_program('dpkg-architecture', required: false, native: true)
tar_prog = find_program('tar', required: false, native: true)
ar_prog  = find_program('ar', required: false, native: true)
if dpkg_architecture_prog.found() and tar_prog.found() and ar_prog.found()
	# Get the "Architecture:" field using dpkg-achitecture
	dpkg_architecture_run = run_command(dpkg_architecture_prog,'-q','DEB_HOST_ARCH', check: true)
	
	debian_depends = ['libc6', 'libstdc++6']
	debian_depends_list = {
		# 'dependency_name': 'Debian package name'
		'curl'             : 'libcurl4',
		'sdl2'             : 'libsdl2-2.0-0',
		'sdl2_image'       : 'libsdl2-image-2.0-0',
		'sdl2_ttf'         : 'libsdl2-ttf-2.0-0',
		'sqlite3'          : 'libsqlite3-0',
		'inih'             : 'libinireader0',
	}
	if enable_native_progs
		foreach dep_name, package_name: debian_depends_list
			if not get_variable('dep_'+dep_name+'_static')
				if config_h.has('ARCOLLECT_VERREQ_'+dep_name)
					debian_depends += package_name+' ('+config_h.get('ARCOLLECT_VERREQ_'+dep_name,'')+')'
				else
					debian_depends += package_name
				endif
			endif
		endforeach
	endif
	
	debian_conf = configuration_data()
	debian_conf.merge_from(config_h)
	debian_conf.set('DEB_HOST_ARCH', dpkg_architecture_run.stdout().strip())
	debian_conf.set('DEB_CONTROL_DEPENDS', ', '.join(debian_depends))
	
	debian_control = configure_file(input: 'control.in', output: 'control', configuration: debian_conf)
	debian_binary  = configure_file(input: 'debian-binary', output: 'debian-binary', copy: true)
	
	debian_control_archive = custom_target('control.tar.gz',
		command: [tar_prog,'-czf','@OUTPUT@','--owner=0','--group=0','-C',meson.current_build_dir(),'./control','./md5sums'],
		output: 'control.tar.gz',
		depends: md5sum_target,
		depend_files: debian_control,
		build_by_default: false,
	)
	debian_data_archive = custom_target('data.tar.xz',
		command: [tar_prog,'-cJf','@OUTPUT@','--owner=0','--group=0','-C',destdir_target.full_path(),'.'],
		output: 'data.tar.xz',
		depends: destdir_target,
		build_by_default: false,
	)
	debian_package = custom_target('arcollect.deb',
		command: [ar_prog,'cr','@OUTPUT@',debian_binary,debian_control_archive,debian_data_archive],
		output: 'arcollect_'+meson.project_version()+'-1_'+debian_conf.get('DEB_HOST_ARCH')+'.deb',
		build_by_default: false,
	)
endif
