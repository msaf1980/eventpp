function(split_linux_lsb_version)
    string(REPLACE "." ";" out ${LSB_RELEASE_VERSION})
    list(GET out 0 version_major)
    list(LENGTH out len)
    if (NOT "${len}" STREQUAL "1")
        list(GET out 1 version_minor)
    endif()

    set(LSB_RELEASE_VERSION_MAJOR "${version_major}" PARENT_SCOPE)
    set(LSB_RELEASE_VERSION_MINOR "${version_minor}" PARENT_SCOPE)
endfunction()

function(get_linux_lsb_release)
    if (NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
        message(FATAL_ERROR "Not a linux")
    endif()
    find_program(LSB_RELEASE_EXEC lsb_release)
    if (NOT LSB_RELEASE_EXEC)
        message(FATAL_ERROR "Could not detect lsb_release executable, can not gather required information")
    endif()

    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --id OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --release OUTPUT_VARIABLE LSB_RELEASE_VERSION_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --codename OUTPUT_VARIABLE LSB_RELEASE_CODENAME_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)

    set(LSB_RELEASE_ID "${LSB_RELEASE_ID_SHORT}" PARENT_SCOPE)
    set(LSB_RELEASE_VERSION "${LSB_RELEASE_VERSION_SHORT}" PARENT_SCOPE)
    set(LSB_RELEASE_CODENAME "${LSB_RELEASE_CODENAME_SHORT}" PARENT_SCOPE)
endfunction()

function(get_linux_os_release)
    if (NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
        message(FATAL_ERROR "Not a linux")
    endif()
    if (NOT EXISTS "/etc/os-release")
        message(FATAL_ERROR "Not found /etc/os-release")
    endif()

    execute_process (
        COMMAND bash -c "awk -F= '/^ID=/{print $2}' /etc/os-release |tr -d '\n' | tr -d '\"'"
        OUTPUT_VARIABLE out
    )
    set(LSB_RELEASE_ID "${out}" PARENT_SCOPE)

    execute_process (
        COMMAND bash -c "awk -F= '/^VERSION_ID=/{print $2}' /etc/os-release |tr -d '\n' | tr -d '\"'"
        OUTPUT_VARIABLE out
    )
    set(LSB_RELEASE_VERSION "${out}" PARENT_SCOPE)

    execute_process (
        COMMAND bash -c "awk -F= '/^VERSION_CODENAME=/{print $2}' /etc/os-release |tr -d '\n' | tr -d '\"'"
        OUTPUT_VARIABLE out
    )
    set(LSB_RELEASE_CODENAME "${out}" PARENT_SCOPE)

endfunction()

function(get_linux_family)
    if (NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
        message(FATAL_ERROR "Not a linux")
    endif()

    if (EXISTS "/etc/redhat-release")
        set(LSB_FAMILY "redhat" PARENT_SCOPE)
    elseif (EXISTS "/etc/debian_version")
        set(LSB_FAMILY "debian" PARENT_SCOPE)
    else()
        set(LSB_FAMILY "unknown" PARENT_SCOPE)
    endif()
endfunction()
