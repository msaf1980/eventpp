set(
    TEST_SOURCES
    main.cpp
    buffer_test.cpp
    dns_resolver_test.cpp
    event_condition_test.cpp
    event_loop_test.cpp
    event_loop_thread_test.cpp
    event_loop_thread_pool_test.cpp
    event_watcher_test.cpp
    http_client_test.cpp
    http_server_test.cpp
    http_trivial_test.cpp
    https_client_test.cpp
    invoke_timer_test.cpp
    sockets_test.cpp
    tcp_client_reconnect_test.cpp
    tcp_client_test.cpp
    tcp_server_test.cpp
    time_test.cpp
    udp_server_test.cpp
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
set_tests_properties(test_eventpp PROPERTIES LABELS "eventpp")

add_executable(test_http_server_fork
    http_server_fork_test.cpp
)
target_link_libraries(test_http_server_fork ${TEST_LIBRARIES})
add_test(
    NAME test_http_server_fork
    COMMAND $<TARGET_FILE:test_http_server_fork>
)
set_tests_properties(test_http_server_fork PROPERTIES LABELS "http_server_fork")
