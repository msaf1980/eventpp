set(EVENTPP_VERSION "0.0.1")

set_version(EVENTPP_VERSION EVENTPP_VERSION_MAJOR EVENTPP_VERSION_MINOR EVENTPP_VERSION_PATCH EVENTPP_VERSION_REVISION)

set(
    EVENTPP_SOURCES
    buffer.cpp
    dns_resolver.cpp
    duration.cpp
    event_loop.cpp
    event_loop_thread.cpp
    event_loop_thread_pool.cpp
    event_watcher.cpp
    fd_channel.cpp
    inner_pre.cpp
    invoke_timer.cpp
    listener.cpp
    sockets.cpp
    tcp/connector.cpp
    tcp/tcp_client.cpp
    tcp/tcp_conn.cpp
    tcp/tcp_server.cpp
    udp/sync_udp_client.cpp
    udp/udp_server.cpp
    utility.cpp
)

include(GNUInstallDirs)

add_library(eventpp STATIC ${EVENTPP_SOURCES})
set_target_properties(eventpp
        PROPERTIES OUTPUT_NAME eventpp
                   POSITION_INDEPENDENT_CODE ON        
)
target_precompile_headers(eventpp PRIVATE pch.h)
install(TARGETS eventpp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(BUILD_SHARED_LIBS)
    add_library(eventpp_shared SHARED ${EVENTPP_SOURCES})
    set_target_properties(
        eventpp_shared
        PROPERTIES OUTPUT_NAME eventpp
                   POSITION_INDEPENDENT_CODE ON
                   SOVERSION "${EVENTPP_VERSION_MAJOR}"
                   VERSION "${EVENTPP_VERSION}")
    target_precompile_headers(eventpp_shared REUSE_FROM eventpp)

    install(TARGETS eventpp_shared
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

# Install headers
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/eventpp"
        DESTINATION "include"
)

configure_file(${PROJECT_SOURCE_DIR}/eventpp.pc.in ${CMAKE_BINARY_DIR}/eventpp.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/eventpp.pc DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
