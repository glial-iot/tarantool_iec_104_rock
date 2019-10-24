if (APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined suppress -flat_namespace")
    INCLUDE_DIRECTORIES(/usr/local/Cellar/json-c/0.13.1/include/json-c)
    LINK_DIRECTORIES(/usr/local/Cellar/json-c/0.13.1/lib)
endif(APPLE)

# Add C library
add_library(lib SHARED lib.c)
target_link_libraries(lib ${MSGPUCK_LIBRARIES} lib60870 json-c)
set_target_properties(lib PROPERTIES PREFIX "" OUTPUT_NAME "lib")

# Install module
install(FILES init.lua DESTINATION ${TARANTOOL_INSTALL_LUADIR}/${PROJECT_NAME}/)
install(TARGETS lib LIBRARY DESTINATION ${TARANTOOL_INSTALL_LIBDIR}/${PROJECT_NAME}/)

add_executable(libtest lib.c)
set_target_properties(libtest PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -DSTANDALONE")
target_link_libraries(libtest ${MSGPUCK_LIBRARIES} lib60870 json-c lua5.1)
