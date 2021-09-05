include(CheckFunctionExists)
include(CheckSymbolExists)

set(CGET_DIR "${CMAKE_BINARY_DIR}" CACHE STRING "Cget work dir")

find_package(PkgConfig REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED) # Threads::Threads
set(CMAKE_REQUIRED_LIBRARIES Threads::Threads) # required libs for check_function_exists

if (BUILD_TESTING)
    check_symbol_exists(pthread_barrier_init pthread.h HAVE_PTHREAD_BARRIER) # for MacOS
    check_symbol_exists(pthread_spin_init pthread.h HAVE_PTHREAD_SPIN) # for MacOS
endif()

if ("${BUILD_DEPS}" STREQUAL "off")
    # Use libraries from OS
else()
    # Init cget
    cget_init(CGET_INIT_CMD "${BUILD_DEPS}" "${CGET_DIR}")
    cget_install(CGET_INSTALL_CMD  "${CMAKE_SOURCE_DIR}/requirements.txt" "${CGET_DIR}")

    if (NOT DEFINED CMAKE_PREFIX_PATH)
        set(CMAKE_PREFIX_PATH "${CGET_DIR}/cget")
    endif()
endif() # BUILD_DEPS

#pkg_check_modules(LIBCONCURRENT REQUIRED libconcurrent>=0.0.1)
#include_directories(${LIBCONCURRENT_INCLUDE_DIRS})
#link_directories(${LIBCONCURRENT_LIBRARY_DIRS})
#link_libraries(concurrent)
