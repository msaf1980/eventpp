if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Plain")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    set(COVERAGE ON)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(allowedBuildTypes Plain Debug ASanDebug TSanDebug Release ASan TSan RelWithDebInfo MinSizeRel)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

if (COVERAGE)
    message(FATAL_ERROR "coverage not implemented")
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)

if(CMAKE_BUILD_TYPE MATCHES "ASan")
    set(ENABLE_ASAN ON)
endif()
if(CMAKE_BUILD_TYPE MATCHES "TSan")
    set(ENABLE_TSAN ON)
endif()

if (ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ASAN and TSAN can't be used together")
endif()

if(EXPORT_COMPILE)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()
