add_subdirectory (src)
add_subdirectory (doc)
if(BUILD_TESTING)
   add_subdirectory(test)
   add_subdirectory(benchmark)
endif() # END TEST
