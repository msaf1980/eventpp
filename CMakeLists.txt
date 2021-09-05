cmake_minimum_required(VERSION 3.5)

include (CheckFunctionExists)
include (CheckIncludeFiles)
include (CheckSymbolExists)

file(GLOB cmakeInitFiles cmake_init/*.cmake)
list(SORT cmakeInitFiles)
foreach(cmakeFile ${cmakeInitFiles})
    message("INCLUDE INIT ${cmakeFile}")
    include(${cmakeFile})
endforeach(cmakeFile)

project (libevents)

file(GLOB cmakeFiles cmake/*.cmake)
list(SORT cmakeFiles)
foreach(cmakeFile ${cmakeFiles})
    message("INCLUDE ${cmakeFile}")
    include(${cmakeFile})
endforeach(cmakeFile)

if(BUILD_TESTING)
    include(CTest)
endif()

include_directories (include)

include(dependecies.cmake)
include(add.cmake)

include(${CMAKE_SOURCE_DIR}/options_print.cmake)
