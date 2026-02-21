cmake_minimum_required(VERSION 3.16)

project(WizzManiaServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTS "Build tests" OFF)

# Find system packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Find Boost (for Beast, if you switch later)
find_package(Boost REQUIRED COMPONENTS system)

# MySQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# nlohmann-json
find_package(nlohmann_json 3.2.0 REQUIRED)

# ============================================
# Server Executable
# ============================================
if(NOT BUILD_TESTS)
    set(SERVER_SOURCES
        sources/initialization_controller.cpp
        sources/auth/auth_controller.cpp
        sources/auth/auth_service.cpp
        sources/user/user_controller.cpp
        sources/user/user_service.cpp
        sources/server.cpp
        sources/database/database_channels.cpp
        sources/database/database_core.cpp
        sources/database/database_invitations.cpp
        sources/database/database_messages.cpp
        sources/database/database_users.cpp
        sources/database/database.cpp
        sources/helpers.cpp
        sources/message/message_controller.cpp
        sources/message/message_service.cpp
        sources/invitation/invitation_controller.cpp
        sources/invitation/invitation_service.cpp
        sources/channel/channel_controller.cpp
        sources/channel/channel_service.cpp
        sources/websocket_manager.cpp
        ../common/utils.cpp  
        )

    set(SERVER_HEADERS
        headers/initialization_controller.hpp
        headers/auth/auth_controller.hpp
        headers/auth/auth_service.hpp
        headers/user/user_controller.hpp
        headers/user/user_service.hpp
        headers/server.hpp
        headers/database.hpp
        headers/exception.hpp
        headers/helpers.hpp
        headers/message/message_controller.hpp
        headers/message/message_service.hpp
        headers/invitation/invitation_controller.hpp
        headers/invitation/invitation_service.hpp
        headers/channel/channel_controller.hpp
        headers/channel/channel_service.hpp
        headers/json_helpers.hpp
        headers/websocket_manager.hpp
    )

    add_executable(wizzmania-server ${SERVER_SOURCES})

    target_include_directories(wizzmania-server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/sources 
        ${CMAKE_CURRENT_SOURCE_DIR}/headers
         ${CMAKE_CURRENT_SOURCE_DIR}/headers/auth
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/user
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/channel
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/invitation
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/message
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
        ${MYSQL_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
    )

    target_link_libraries(wizzmania-server PRIVATE
        nlohmann_json::nlohmann_json
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        ${MYSQL_LIBRARIES}
        ${Boost_LIBRARIES}
        mysqlcppconn
    )

    target_compile_options(wizzmania-server PRIVATE
        -Wall -Wextra -Wpedantic
    )
    
    target_compile_definitions(wizzmania-server PRIVATE
        CROW_ENABLE_SSL
        CROW_USE_BOOST
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
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
        ${Boost_INCLUDE_DIRS}
    )
    
    target_link_libraries(tests PRIVATE
        nlohmann_json::nlohmann_json
        Threads::Threads
        ${Boost_LIBRARIES}
    )
endif()