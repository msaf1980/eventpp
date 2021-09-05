string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSTEM_PROCESSOR)
if (${SYSTEM_PROCESSOR} MATCHES "amd64"
    OR ${SYSTEM_PROCESSOR} MATCHES "x86_64"
    OR ${SYSTEM_PROCESSOR} MATCHES "i386"
    OR ${SYSTEM_PROCESSOR} MATCHES "x86")
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(ARCH_TYPE x86_64)
    else()
        set(ARCH_TYPE i386)
    endif()
elseif(${CSYSTEM_PROCESSOR} MATCHES "(arm)|(aarch")
    if (CMAKE_CL_64)
        set(ARCH_TYPE arm64)
    else()
        set(ARCH_TYPE arm)
    endif()
elseif(${CSYSTEM_PROCESSOR} MATCHES "mips") # Untested
    if (CMAKE_CL_64)
        set(ARCH_TYPE mips64)
    else()
        set(ARCH_TYPE mips)
    endif()
elseif(${CSYSTEM_PROCESSOR} MATCHES "(ppc)|(power)") # Untested
    if (CMAKE_CL_64)
        set(ARCH_TYPE ppc64)
    else()
        set(ARCH_TYPE ppc32)
    endif()
elseif(${CSYSTEM_PROCESSOR} MATCHES "riscv64") # Untested....
    set(ARCH_TYPE riscv64)
elseif(${CSYSTEM_PROCESSOR} MATCHES "s390x") # Untested....
    set(ARCH_TYPE s390x)
else()
    set(ARCH_TYPE "${CSYSTEM_PROCESSOR}")
    #message(FATAL_ERROR "arch ${CMAKE_SYSTEM_PROCESSOR} not supported")
endif()

add_definitions(-DCARCH_${ARCH_TYPE}=1)
