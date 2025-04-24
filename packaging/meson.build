packages_files = [
]

packages_conf = configuration_data()
packages_conf.merge_from(config_h)
packages_conf.set('MESON_BUILD_ROOT', meson.project_build_root())
packages_conf.set('MESON_CURRENT_BUILD_DIR', meson.current_build_dir())
packages_conf.set('MESON_SOURCE_ROOT', meson.project_source_root())

sh_prog = find_program('sh', required: false, native: true)

# Strip targets
if get_option('strip')
	strip_exe = find_program('strip', native: true, disabler: true)
	install_arcollect_exe = custom_target('arcollect_exe_stripped', command: [strip_exe,'-o','@OUTPUT0@','@INPUT0@'], input: arcollect_exe, output: 'arcollect.strip.exe')
else
	install_arcollect_exe = arcollect_exe
endif

# DESTDIR target
# This special target run `meson compile` to a DESTDIR
destdir_depends = []
destdir_depend_files = []
if enable_native_progs
	destdir_depends += [webext_adder_exe,install_arcollect_exe]
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
		'fontconfig'       : ['libfontconfig-dev'       ,'libfontconfig'],
		'freetype2'        : ['libfreetype-dev'         ,'libfreetype'],
		'harfbuzz'         : ['libharfbuzz-dev'         ,'libharfbuzz[0-9a-z]+'],
		'inih'             : ['libinih-dev'             ,'libinireader'],
		'lcms2'            : ['liblcms2-dev'            ,'liblcms2'],
		'libbrotlicommon'  : ['libbrotli-dev'           ,'libbrotli'],
		'libcurl'          : ['libcurl4-openssl-dev'    ,'libcurl4'],
		'libjpeg'          : ['libjpeg-turbo8-dev'      ,'libjpeg-turbo8'], # TODO Don't assume libjpeg-turbo62, guess from libjpeg
		'libpng'           : ['libpng-dev'              ,'libpng'],
		'libtiff-4'        : ['libtiff5-dev'            ,'libtiff5'], # That is not an error: libtiff5 provide libtiff-4
		'OpenImageIO'      : ['libopenimageio-dev'      ,'libopenimageio2.4'],
		'sdl2'             : ['libsdl2-dev'             ,'libsdl2'],
		'sqlite3'          : ['libsqlite3-dev'          ,'libsqlite3'],
		'zlib'             : ['zlib1g-dev'              ,'zlib1g'],
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
wix_prog = find_program('wix', required: false, version: '>='+config_h.get('ARCOLLECT_WIX_MIN_VERSION'))

copying_rtf = custom_target('COPYING.rtf',
	input: '../COPYING',
	output: 'COPYING.rtf',
	command: find_program('copying_to_rtf.py', required: true, native: true),
	capture: true, feed: true
)

wix_config = custom_target('wix-config',
	input: [
		config_h_file,
		# Match these input to keys a few lines below -↓
		install_arcollect_exe,
		webext_appmanifest_firefox_windows,
		webext_adder_exe,
		copying_rtf,
	],
	output: 'config.wxi',
	command: [find_program('generate-wix-config.py', required: true, native: true),
		','.join([
			# Just there --------------------------------↑
			'built_arcollect_exe',
			'built_mozilla_manifest',
			'built_webext_adder_exe',
			'copying_rtf_path',
		]),'@INPUT@'
	],
	capture: true, build_by_default: false,
)

if wix_prog.found() and enable_native_progs
	# WiX depends
	wix_deps = [
		wix_config,
	]
	wix_dep_files = files(
	)+[
	]
	
	# Auto-configuration
	if host_machine.cpu_family() == 'x86'
		candle_arch = 'x86'
	elif host_machine.cpu_family() == 'x86_64'
		candle_arch = 'x64'
	elif host_machine.cpu_family() == 'aarch64'
		candle_arch = 'arm64'
	else
		candle_arch = host_machine.cpu_family()
		warning('Unknow CPU family '+candle_arch+'. MSI creation should fail')
	endif
	
	wix_args = [
		wix_prog,
		'build',
		'-arch',candle_arch,
		'-defaultcompressionlevel','high',
		'-ext','WixToolset.UI.wixext',
		'-cabcache','@PRIVATE_DIR@',
		'-intermediatefolder', '@PRIVATE_DIR@',
		'-out','@OUTPUT0@',
		'-pdb','@OUTPUT1@',
		'-pdbtype','full',
		'-d','config_wxi="'+wix_config.full_path()+'"',
		'-loc','@INPUT1@',
		'@INPUT0@',
	]
	
	base_msi = custom_target('arcollect-en_US.msi', command: [wix_args], input: ['arcollect.wxs',msi_wlxs['C']], output: ['arcollect-en_US.msi','arcollect-en_US.msi.wixpdb'], depends: wix_deps, depend_files: wix_dep_files)
	
	msi_targets = [
		base_msi,
	]
	foreach culture: msi_wlx_cultures
		lang_msi = custom_target('arcollect-'+culture+'.msi',
			command: [wix_args,'-culture',culture],
			input: ['arcollect.wxs',msi_wlxs[culture]],
			output: ['arcollect-'+culture+'.msi','arcollect-'+culture+'.msi.wixpdb'],
			depends: wix_deps, depend_files: wix_dep_files,
		)
		msi_targets += [lang_msi]
	endforeach
	alias_target('all-msi',msi_targets)
endif
