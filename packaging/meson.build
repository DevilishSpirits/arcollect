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
destdir_prefix = destdir_target.full_path()#+get_option('prefix')
# FIXME This is a workaround for Windows build
# TODO Checkout exact Meson behavior
if get_option('prefix')[0] == '/'
	destdir_prefix += get_option('prefix')
endif

# destdir_md5sum
# This target run md5sum on DESTDIR (destdir_target)
if sh_prog.found()
	md5sum_target = custom_target('destdir_md5sum',
		command: [sh_prog,'-c','cd \''+destdir_target.full_path()+'\' && find usr -type f | exec xargs md5sum'],
		output: 'md5sums',
		capture: true,
		depends: destdir_target,
		build_by_default: false,
	)
endif

# ArchLinux PKGBUILD
# Two favlors are generated:
# 	PKGBUILD usable by anybody
# 	PKGBUILD.local that use local directory as source directory
pkgbuild_depends = ['\'xdg-utils\'']
pkgbuild_depends_list = {
	# 'dependency_name': 'ArchLinux package name'
	'boost'            : 'boost',
	'curl'             : 'curl',
	'dbus-1'           : 'dbus',
	'freetype2'        : 'freetype2',
	'harfbuzz'         : 'harfbuzz',
	'inih'             : 'libINIReader.so',
	'lcms2'            : 'lcms2',
	'OpenImageIO'      : 'openimageio',
	'sdl2'             : 'sdl2',
	'sqlite3'          : 'sqlite',
}
foreach dep_name, package_name: pkgbuild_depends_list
	if dep_name in system_dependencies
		pkgbuild_depends += '\''+package_name+config_h.get('ARCOLLECT_VERREQ_'+dep_name,'')+'\''
	endif
endforeach

pkgbuild_conf = configuration_data()
pkgbuild_conf.merge_from(config_h)
pkgbuild_conf.set_quoted('PKGBUILD_SOURCE', 'https://github.com/DevilishSpirits/arcollect/archive/refs/tags/v$pkgver.tar.gz')
pkgbuild_conf.set('PKGBUILD_DEPENDS', ' '.join(pkgbuild_depends))
pkgbuild_conf.set_quoted('PKGBUILD_MD5SUMS', 'SKIP')
pkgbuild_conf.set('PKGBUILD_BUILDDIR', 'PKGBUILD.build')
pkgbuild_conf.set('PKGBUILD_MESON_COMMAND', 'meson --prefix=/usr --buildtype=release -Db_lto=true -Db_pie=true --libexecdir lib "$srcdir/arcollect-$pkgver" PKGBUILD.build')
configure_file(input: 'PKGBUILD.in', output: 'PKGBUILD', configuration: pkgbuild_conf)

pkgbuild_local_conf = configuration_data()
pkgbuild_local_conf.merge_from(config_h)
pkgbuild_local_conf.set('PKGBUILD_SOURCE', '')
pkgbuild_local_conf.set('PKGBUILD_DEPENDS', ' '.join(pkgbuild_depends))
pkgbuild_local_conf.set('PKGBUILD_MD5SUMS', '')
pkgbuild_local_conf.set('PKGBUILD_BUILDDIR', meson.project_build_root())
pkgbuild_local_conf.set('PKGBUILD_MESON_COMMAND', 'meson configure \''+meson.project_build_root()+'\' --prefix=/usr --libexecdir lib # Enforce a correct configurations')
pkgbuild_local = configure_file(input: 'PKGBUILD.in', output: 'PKGBUILD.local', configuration: pkgbuild_local_conf)

foreach packages_file: packages_files
	configure_file(input: packages_file+'.in', output: packages_file, configuration: packages_conf)
endforeach

# Debian
# FIXME Generated a binary package by hand is really dirty !
# The Debian package generation consist of the generation a binary package
# manually.
apt_prog = find_program('apt', required: false, native: true)
dpkg_architecture_prog = find_program('dpkg-architecture', required: false, native: true)
tar_prog = find_program('tar', required: false, native: true)
ar_prog  = find_program('ar', required: false, native: true)
if apt_prog.found() and dpkg_architecture_prog.found() and tar_prog.found() and ar_prog.found() and sh_prog.found()
	# Get the "Architecture:" field using dpkg-achitecture
	dpkg_architecture_run = run_command(dpkg_architecture_prog,'-q','DEB_HOST_ARCH', check: true)
	deb_system_dependencies = system_dependencies.keys()
	deb_skip_dependencies = []
	
	debian_depends = ['libc6', 'libstdc++6', 'xdg-utils']
	debian_depends_list = {
		# 'dependency_name': ['Debian -dev package name','package-regex'[0-9\-\.]*]
		'dbus-1'           : ['libdbus-1-dev'           ,'libdbus-1'],
		'freetype2'        : ['libfreetype-dev'         ,'libfreetype'],
		'harfbuzz'         : ['libharfbuzz-dev'         ,'libharfbuzz[0-9a-z]+'],
		'inih'             : ['libinih-dev'             ,'libinireader'],
		'lcms2'            : ['liblcms2-dev'            ,'liblcms2'],
		'libcurl'          : ['libcurl4-openssl-dev'    ,'libcurl4'],
		'OpenImageIO'      : ['libopenimageio-dev'      ,'libopenimageio'],
		'sdl2'             : ['libsdl2-dev'             ,'libsdl2'],
		'sqlite3'          : ['libsqlite3-dev'          ,'libsqlite3'],
	}
	# Boost
	if 'boost' in deb_system_dependencies
		# Detect Boost version
		deb_boostver = dependency('boost').version().split('.')
		deb_boostver = deb_boostver[0]+'.'+deb_boostver[1] # Debian only keeps major.minor
		# Add boost libs
		deb_boostlibs = [
			#'boost-filesystem', # See below
			'boost-thread',
		]
		foreach boostlib: deb_boostlibs
			deb_system_dependencies += boostlib
			dep_name = 'lib'+boostlib+deb_boostver
			debian_depends_list += {boostlib: [dep_name+'-dev',dep_name]}
		endforeach
		# We have a straying 'boost' in deb_system_dependencies, we put a random dep
		# on it to avoid configuration breakage
		dep_name = 'libboost-filesystem'+deb_boostver
		debian_depends_list += {'boost': [dep_name+'-dev',dep_name]}
	endif
	# Detect non-dev package name
	foreach dep: deb_system_dependencies
		pkg_name = debian_depends_list[dep]
		result = run_command(sh_prog,'-c','\''+apt_prog.full_path()+'\' show \''+pkg_name[0]+'\' | grep -F \'Depends:\' -m 1 | exec grep -oE \''+pkg_name[1]+'[0-9\.-]*(\ |$)\' -m 1')
		if result.returncode() == 0
			debian_depends_list += {dep: result.stdout().strip()}
		else
			debian_depends_list += {dep: pkg_name[1]}
		endif
	endforeach
	
	if enable_native_progs
		foreach dep_name, package_name: debian_depends_list
			if dep_name in deb_system_dependencies
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

# Windows
candle_prog = find_program('candle', required: false, version: '>=3')
light_prog = find_program('light', required: false)

copying_rtf = custom_target('COPYING.rtf',
	input: '../COPYING',
	output: 'COPYING.rtf',
	command: find_program('copying_to_rtf.py', required: true, native: true),
	capture: true, feed: true
)

if candle_prog.found() and light_prog.found() and enable_native_progs
	# WiX depends
	wix_deps = [
		destdir_target,
		arcollect_exe,
		copying_rtf,
		webext_adder_exe,
	]
	wix_dep_files = files(
	)+[
		webext_appmanifest_firefox_windows,
	]
	# Configuration
	wix_conf = configuration_data()
	wix_conf.merge_from(config_h)
	arcollect_exe_basename    = arcollect_exe.full_path().split('/')[-1].split('\\')[-1]
	webext_adder_exe_basename = webext_adder_exe.full_path().split('/')[-1].split('\\')[-1]
	wix_conf.set('built_arcollect_exe',destdir_prefix/get_option('bindir')/arcollect_exe_basename)
	wix_conf.set('built_webext_adder_exe',destdir_prefix/get_option('libexecdir')/webext_adder_exe_basename)
	wix_conf.set('built_mozilla_manifest',webext_appmanifest_firefox_windows_full_path) # webext_appmanifest_firefox_windows
	wix_conf.set('copying_rtf_path',copying_rtf.full_path())
	
	# Auto-configuration
	if host_machine.cpu_family() == 'x86'
		candle_arch = 'x86'
		wix_conf.set('WIX_PFiles','ProgramFilesFolder')
	elif host_machine.cpu_family() == 'x86_64'
		candle_arch = 'x64'
		wix_conf.set('WIX_PFiles','ProgramFiles64Folder')
	elif host_machine.cpu_family() == 'ia64'
		candle_arch = 'ia64'
		wix_conf.set('WIX_PFiles','ProgramFiles64Folder')
	else # Likely won't works outside x86 and ia64
		candle_arch = host_machine.cpu_family()
		wix_conf.set('WIX_PFiles','ProgramFilesFolder')
		warning('Unknow CPU family '+candle_arch+'. MSI creation should fail')
	endif
	
	wix_args = [
		'-pedantic',
	]
	foreach key: wix_conf.keys()
		wix_args += ['-d'+key+'='+wix_conf.get(key)]
	endforeach
	
	candle_args = wix_args+[
		'-arch',candle_arch,
		'-out','@OUTPUT@',
		'@INPUT@'
	]
	
	candle = generator(candle_prog, arguments: candle_args, output: ['@BASENAME@.wixobj'])
	
	light_args = wix_args+[
		'-sval', # Needed under Wine
		'-ext', 'WixUIExtension',
		'-out','@OUTPUT@',
		'@INPUT@'
	]
	custom_target('arcollect.msi', command: [light_prog, light_args], input: candle.process('arcollect.wxs'), output: 'arcollect.msi', depends: wix_deps, depend_files: wix_dep_files)
endif
