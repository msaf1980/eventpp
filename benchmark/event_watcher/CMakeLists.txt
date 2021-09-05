set(TEST_LIBRARIES Threads::Threads event)
if(BUILD_SHARED_LIBS)
    list(APPEND TEST_LIBRARIES eventpp_shared)
else()
    list(APPEND TEST_LIBRARIES eventpp)
endif()

add_executable(benchmark_event_watcher event_watcher.cpp)
target_link_libraries(benchmark_event_watcher ${TEST_LIBRARIES})
