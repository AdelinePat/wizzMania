cmake_minimum_required(VERSION 3.16)

project(WizzManiaServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTS "Build tests" OFF)

# Find MySQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# Find JSON library (nlohmann-json)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Common source files (shared with client)
# set(COMMON_SOURCES
#     ../common/protocol.cpp
#     ../common/message.cpp
# )

# ============================================
# Server Executable
# ============================================
if(NOT BUILD_TESTS)
    set(SERVER_SOURCES
        server.cpp
        # database.cpp
        # client_handler.cpp
        # ${COMMON_SOURCES}
    )

    add_executable(wizzmania-server ${SERVER_SOURCES})

    target_include_directories(wizzmania-server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
        ${MYSQL_INCLUDE_DIRS}
    )

    target_link_libraries(wizzmania-server PRIVATE
        ${MYSQL_LIBRARIES}
        nlohmann_json::nlohmann_json
        pthread
    )

    target_compile_options(wizzmania-server PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

# ============================================
# Tests Target
# ============================================
if(BUILD_TESTS)
    add_executable(tests
        ../tests/test_main.cpp
        ../tests/test_protocol.cpp
        ../tests/test_message.cpp
        ${COMMON_SOURCES}
    )
    
    target_include_directories(tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
    )
    
    target_link_libraries(tests PRIVATE
        nlohmann_json::nlohmann_json
        pthread
    )
endif()