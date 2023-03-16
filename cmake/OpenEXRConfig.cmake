if (NOT OPENEXR_FOUND)
	message(NOTICE "Using Arcollect injected OpenEXRConfig.cmake (inject Imath @IMATH_VERSION_STRING@ only)")
	# Imath stuff
	set(IMATH_INCLUDES "@IMATH_INCLUDES@")
	set(ILMBASE_INCLUDES "")
	set(ILMBASE_LIBRARIES "")
	set(IMATH_VERSION_STRING "@IMATH_VERSION_STRING@")
	# OpenEXR stuff 
	set(OPENEXR_INCLUDES "${IMATH_INCLUDES}")
	set(OPENEXR_LIBRARIES "")
	set(OPENEXR_VERSION "3.fake") # Fake 3.x to make OIIO choose "Imath/Imath*" includes
endif()
