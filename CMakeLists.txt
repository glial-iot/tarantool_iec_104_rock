cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(ckit C CXX)
# project(ckit C CXX)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ${CMAKE_MODULE_PATH})

# feature checks
include(CheckLibraryExists)
check_library_exists(rt clock_gettime "time.h" CONFIG_SYSTEM_HAS_CLOCK_GETTIME)

# check if we are on a little or a big endian
include (TestBigEndian)
test_big_endian(PLATFORM_IS_BIGENDIAN)

#Hardware abstraction layer (HAL) config
option(BUILD_HAL "Build the platform abstraction layer (HAL)" ON)
option(BUILD_COMMON "Build common code (shared with other libraries - e.g. libiec61850)" ON)

if(BUILD_HAL)

if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/dependencies/mbedtls-2.6.0)
set(WITH_MBEDTLS 1)
endif(EXISTS ${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/dependencies/mbedtls-2.6.0)

endif(BUILD_HAL)

# Find Tarantool and Lua dependecies
set(TARANTOOL_FIND_REQUIRED ON)
find_package(Tarantool)
include_directories(${TARANTOOL_INCLUDE_DIRS})

# Find MsgPuck llibrary
set(MsgPuck_FIND_REQUIRED ON)
find_package(MsgPuck)
include_directories(${MSGPUCK_INCLUDE_DIRS})

# Find other dependecies

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/config
    ${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api
    ${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/internal
    ${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/common/inc
    ${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/hal/inc
)

if(WITH_MBEDTLS)
include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/src/hal/tls/mbedtls
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/mbedtls-2.6.0/include
)

file(GLOB tls_SRCS ${CMAKE_CURRENT_LIST_DIR}/dependencies/mbedtls-2.6.0/library/*.c)

add_definitions(-DCONFIG_CS104_SUPPORT_TLS=1)
add_definitions(-DMBEDTLS_CONFIG_FILE="mbedtls_config.h")

endif(WITH_MBEDTLS)

set(API_HEADERS
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/hal/inc/hal_time.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/hal/inc/hal_thread.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/hal/inc/hal_socket.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/hal/inc/hal_serial.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/hal/inc/tls_config.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api/cs101_master.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api/cs101_slave.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api/cs104_slave.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api/iec60870_master.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api/iec60870_slave.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api/iec60870_common.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api/cs101_information_objects.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api/cs104_connection.h
	${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src/inc/api/link_layer_parameters.h
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib60870/lib60870-C/src)

INSTALL(FILES ${API_HEADERS} DESTINATION include/lib60870 COMPONENT Development)

# Set CFLAGS
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra")
# Set CXXFLAGS
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")

# Build module
add_subdirectory(ckit)

enable_testing()

set (LUA_PATH "LUA_PATH=${PROJECT_SOURCE_DIR}/?.lua\\;${PROJECT_SOURCE_DIR}/?/init.lua\\;;")

add_test(ckit ${CMAKE_SOURCE_DIR}/test/ckit.test.lua)
set_tests_properties(ckit PROPERTIES ENVIRONMENT "${LUA_PATH}")

# Add `make check`
add_custom_target(check
    WORKING_DIRECTORY ${PROJECT_BUILD_DIR}
    COMMAND ctest -V)
