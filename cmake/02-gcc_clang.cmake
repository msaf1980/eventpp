if(CMAKE_COMPILER_IS_GNU OR CMAKE_COMPILER_IS_CLANG)
	if(NOT CMAKE_BUILD_TYPE STREQUAL "Plain")
		set(ALL_C_FLAGS
			-Wall
			-Wextra
			-W
			-Wpedantic
			-Wconversion
			-Wdeclaration-after-statement
			-Wwrite-strings
		)

		set(ALL_CXX_FLAGS
			-Wall
			-Wextra
			-W
			-Wpedantic
			-Wconversion
			-Wwrite-strings
		)

		if(CMAKE_COMPILER_IS_GNU)
			# some warnings we want are not available with old GCC versions
			if (CMAKE_C_COMPILER_VERSION VERSION_GREATER 4.5 OR CMAKE_C_COMPILER_VERSION VERSION_EQUAL 4.5)
				list(APPEND ALL_C_FLAGS "-Wlogical-op")
				list(APPEND ALL_CXX_FLAGS "-Wlogical-op")
			endif()
			if (CMAKE_C_COMPILER_VERSION VERSION_GREATER 4.8 OR CMAKE_C_COMPILER_VERSION VERSION_EQUAL 4.8)
				list(APPEND ALL_C_FLAGS "-Wshadow")
				list(APPEND ALL_CXX_FLAGS "-Wshadow")
			endif()
		endif(CMAKE_COMPILER_IS_GNU)
			
		foreach(cFlag ${ALL_C_FLAGS})
			append_flag(CMAKE_C_FLAGS_${BUILD_TYPE} ${cFlag})
		endforeach(cFlag)
		foreach(cxxFlag ${ALL_CXX_FLAGS})
			append_flag(CMAKE_CXX_FLAGS_${BUILD_TYPE} ${cxxFlag})
		endforeach(cxxFlag)
	endif()

	# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# endif()
	# if(CMAKE_BUILD_TYPE STREQUAL "Release")
	# endif()
	if(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		append_flag(CMAKE_C_FLAGS_${BUILD_TYPE} "-g")
		append_flag(CMAKE_CXX_FLAGS${BUILD_TYPE} "-g")
	endif()

	if(CMAKE_BUILD_TYPE MATCHES "Debug")
		# Disable optimimization
		append_flag(CMAKE_C_FLAGS_${BUILD_TYPE} "-O0")
		append_flag(CMAKE_CXX_FLAGS_${BUILD_TYPE} "-O0")
		#append_flag(CMAKE_C_FLAGS_${BUILD_TYPE} "-fno-inline")
		#append_flag(CMAKE_CXX_FLAGS_${BUILD_TYPE} "-fno-inline")
		append_flag(CMAKE_C_FLAGS_${BUILD_TYPE} "-g")
		append_flag(CMAKE_CXX_FLAGS_${BUILD_TYPE} "-g")
		add_definitions(-DDEBUG)
	endif()

	# Profile
	if(PROFILE)
		append_flag(CMAKE_C_FLAGS_${BUILD_TYPE} "-g")
		append_flag(CMAKE_CXX_FLAGS_${BUILD_TYPE} "-g")
		append_flag(CMAKE_EXE_LINKER_FLAGS_${BUILD_TYPE} "-lprofiler")
	endif()
endif(CMAKE_COMPILER_IS_GNU OR CMAKE_COMPILER_IS_CLANG)
