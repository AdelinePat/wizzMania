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
    
    # Find Qt6
    find_package(Qt6 REQUIRED COMPONENTS 
        Core 
        Gui 
        Widgets 
        Network
    )
endif()

# Common source files (pure C++ - no Qt!)
# set(COMMON_SOURCES
#     ../common/protocol.cpp
#     ../common/message.cpp
# )

# ============================================
# Client Executable
# ============================================
if(NOT BUILD_TESTS)
    set(CLIENT_SOURCES
        Sources/client.cpp
        Sources/mainwindow.cpp
        Sources/loginwidget.cpp
    )

    set(CLIENT_HEADERS
        Headers/mainwindow.h
        Headers/loginwidget.h
    )

    set(CLIENT_FORMS
        Forms/mainwindow.ui
        Forms/loginwidget.ui
    )

    add_executable(wizzmania-client WIN32 
        ${CLIENT_SOURCES}
        ${CLIENT_HEADERS}
        ${CLIENT_FORMS}
    )

    # Tell AUTOUIC where to find .ui files
    set_target_properties(wizzmania-client PROPERTIES
        AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/Forms"
    )

    target_include_directories(wizzmania-client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
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

# ============================================
# Tests Target (Linux native - no Qt)
# ============================================
if(BUILD_TESTS)
    # Find JSON for tests
    find_package(nlohmann_json 3.2.0 REQUIRED)
    
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
