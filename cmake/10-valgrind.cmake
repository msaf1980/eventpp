# For run tests under valgrind:
# ctest -T memcheck
# or run target test_memcheck
# cmake --build . -v --target test_memcheck
# or run target test_memcheck with comfigured backend
# make test_memcheck
# or
# ninja test_memcheck
if (ENABLE_VALGRIND)
    find_program(MEMORYCHECK_COMMAND valgrind)
    add_definitions(-DVALGRIND_ENABLED=1)
    set(MEMORYCHECK_SUPPRESS "${PROJECT_SOURCE_DIR}/valgrind_suppress.txt" )
    set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full --track-origins=yes --show-reachable=yes --error-exitcode=255 --suppressions=${MEMORYCHECK_SUPPRESS}" )
    set(MEMCHECK_LOGFILE memcheck.log)
    add_custom_target(test-valgrind
        COMMAND ctest -O ${MEMCHECK_LOGFILE} -t memcheck
        COMMAND tail -n1 ${MEMCHECK_LOGFILE} | grep 'Memory checking results:' > /dev/null
        COMMAND rm -f ${MEMCHECK_LOGFILE}
        DEPENDS ${DART_CONFIG}
    )
    add_custom_target(test_memcheck
        COMMAND ${CMAKE_CTEST_COMMAND} --force-new-ctest-process -O ${MEMCHECK_LOGFILE} --test-action memcheck
        COMMAND cat ${MEMCHECK_LOGFILE}
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    )
endif()
