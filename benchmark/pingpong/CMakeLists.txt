find_package(Boost REQUIRED COMPONENTS system thread)

set(TEST_LIBRARIES eventpp event Threads::Threads)

add_executable(eventpp_pp client.cpp)
target_link_libraries(eventpp_pp ${TEST_LIBRARIES})

add_executable(eventpp_fixed_size_pp client_fixed_size.cpp)
target_link_libraries(eventpp_fixed_size_pp ${TEST_LIBRARIES})
