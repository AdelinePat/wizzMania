cmake_minimum_required(VERSION 3.16)

project(WizzManiaClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTS "Build tests" OFF)

# Qt settings (only if not building tests)
if(NOT BUILD_TESTS)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    
    # Find Qt6 (toolchain file specifies which Qt to use)
    find_package(Qt6 REQUIRED COMPONENTS 
        Core 
        Gui 
        Widgets 
        Network
    )
endif()

# ============================================
# Client Executable
# ============================================
if(NOT BUILD_TESTS)
    set(CLIENT_SOURCES
        client.cpp
    )

    add_executable(wizzmania-client WIN32 ${CLIENT_SOURCES})

    target_include_directories(wizzmania-client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../common
    )

    target_link_libraries(wizzmania-client PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
    )

    # Windows-specific settings
    if(WIN32)
        set_target_properties(wizzmania-client PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
    endif()

    target_compile_options(wizzmania-client PRIVATE
        -Wall -Wextra
    )
endif()