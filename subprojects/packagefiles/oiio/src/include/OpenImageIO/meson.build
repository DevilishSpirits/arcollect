project_version = meson.project_version().split('.')
oiioversion_h_conf = configuration_data()
oiioversion_h_conf.set('CMAKE_CXX_STANDARD',17)
oiioversion_h_conf.set('PROJECT_VERSION_MAJOR',project_version[0])
oiioversion_h_conf.set('PROJECT_VERSION_MINOR',project_version[1])
oiioversion_h_conf.set('PROJECT_VERSION_PATCH',project_version[2])
oiioversion_h_conf.set('PROJECT_VERSION_RELEASE_TYPE','Release')
oiioversion_h_conf.set('PROJECT_VERSION_TWEAK',project_version[3])
oiioversion_h_conf.set('Imath_VERSION_MAJOR',dep_imath_version[0])
oiioversion_h_conf.set('Imath_VERSION_MINOR',dep_imath_version[1])
oiioversion_h_conf.set('PROJ_NAME','OIIO')
oiioversion_h_conf.set('PROJ_NAMESPACE_V','OIIO_v'+project_version[0]+'_'+project_version[1]+'_'+project_version[2])
oiioversion_h = configure_file(input: 'oiioversion.h.in', output: 'oiioversion.h', configuration: oiioversion_h_conf, format: 'cmake@')

Imath_h_conf = configuration_data()
Imath_h_conf.set('OIIO_USING_IMATH',dep_imath_version[0])
Imath_h = configure_file(input: 'Imath.h.in', output: 'Imath.h', configuration: Imath_h_conf)
half_h = configure_file(input: 'half.h.in', output: 'half.h', configuration: Imath_h_conf)
