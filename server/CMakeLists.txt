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

# Find Crow dependencies
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# ============================================
# Server Executable
# ============================================
if(NOT BUILD_TESTS)
    set(SERVER_SOURCES
        server.cpp
    )

    add_executable(wizzmania-server ${SERVER_SOURCES})

    target_include_directories(wizzmania-server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/vendor/asio/include
        ${CMAKE_SOURCE_DIR}/vendor/crow/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
        ${MYSQL_INCLUDE_DIRS}
    )

    target_link_libraries(wizzmania-server PRIVATE
        ${MYSQL_LIBRARIES}
        nlohmann_json::nlohmann_json
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
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
        ../tests/server/test_main.cpp
        # Add your test files here
    )
    
    target_include_directories(tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/vendor/asio
        ${CMAKE_SOURCE_DIR}/vendor/crow
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
    )
    
    target_link_libraries(tests PRIVATE
        nlohmann_json::nlohmann_json
        pthread
    )
endif()