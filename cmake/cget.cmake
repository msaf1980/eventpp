function(cget_init CGET_CMD build_deps cget_work_dir)
    set(opts
        --cc ${CMAKE_C_COMPILER}
        --cxx ${CMAKE_CXX_COMPILER}
    )
    # default shared/static
    if(build_deps STREQUAL "static" OR build_deps STREQUAL "shared") 
        list(APPEND opts "--${build_deps}")
    endif()

    string(REPLACE ";" " " _opts "${opts}")
    set(${CGET_CMD} "cd ${cget_work_dir} && cget init ${_opts}" PARENT_SCOPE)

    if(NOT EXISTS "${CMAKE_BINARY_DIR}/cget/cget/cget.cmake")
        message("CREATE CGET TOOLCHAIN FILE ${CMAKE_BINARY_DIR}/cget/cget/cget.cmake")
        message("  EXECUTE cd ${cget_work_dir} && cget init ${_opts}")
        execute_process(
            COMMAND cget init ${opts}
            WORKING_DIRECTORY ${cget_work_dir}
            RESULT_VARIABLE status
        )
        if(NOT status EQUAL 0)
            message(FATAL_ERROR "Failed cget init with error: ${status}")
        endif()
    endif()
endfunction ()

function(cget_install CGET_CMD requirements_file cget_work_dir)
    set(opts
        --build-type "${CMAKE_BUILD_TYPE}"
        -f "${requirements_file}"
    )
    string(REPLACE ";" " " _opts "${opts}")
    set(${CGET_CMD} "cd ${cget_work_dir} && cget install ${_opts} -G '${CMAKE_GENERATOR}'" PARENT_SCOPE)
    message("  EXECUTE cd ${cget_work_dir} && cget install ${_opts} -G '${CMAKE_GENERATOR}'")
    execute_process(
        COMMAND cget install ${opts}
            -G "${CMAKE_GENERATOR}"
        WORKING_DIRECTORY ${cget_work_dir}
        RESULT_VARIABLE status
    )
    if(NOT status EQUAL 0)
        message(FATAL_ERROR "Failed cget install with error: ${status}")
    endif()
endfunction ()
