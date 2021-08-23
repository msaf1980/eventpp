set(
    TEST_SOURCES
    main.cpp
    event_loop_test.cpp
    event_loop_thread_test.cpp
    event_loop_thread_pool_test.cpp
    event_watcher_test.cpp
    invoke_timer_test.cpp
    sockets_test.cpp
    time_test.cpp
)

set(TEST_LIBRARIES Threads::Threads event)
if(BUILD_SHARED_LIBS)
    list(APPEND TEST_LIBRARIES eventpp_shared)
else()
    list(APPEND TEST_LIBRARIES eventpp)
endif()

# # Build tests
add_executable(test_eventpp
    ${TEST_SOURCES}
)
target_link_libraries(test_eventpp ${TEST_LIBRARIES})
add_test(
    NAME test_eventpp
    COMMAND $<TARGET_FILE:test_eventpp>
)
set_tests_properties(test_eventpp PROPERTIES LABELS "thpool")
