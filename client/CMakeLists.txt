cmake_minimum_required(VERSION 3.16)

project(WizzManiaClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Tell AUTOUIC where to find .ui files
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/Forms)

# Disable Vulkan requirement (not needed for basic Qt apps)
set(QT_FEATURE_vulkan OFF)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets 
    Network
    WebSockets
)

# Collect source files from Sources/ directory
set(CLIENT_SOURCES
    Sources/client.cpp
    Sources/mainwindow.cpp
    Sources/loginwidget.cpp
    Sources/services/apiclient.cpp
    Sources/services/authmanager.cpp
    Sources/services/serverconfig.cpp
    Sources/ws/websocket_client.cpp
    Sources/utils/message_json.cpp
    Sources/models/messagemodel.cpp
    Sources/models/channelmodel.cpp
    Sources/widgets/messageitemwidget.cpp
    Sources/widgets/channelrowwidget.cpp
    Sources/widgets/channelpanelwidget.cpp
    Sources/widgets/rightpanelwidget.cpp
)

# Collect header files from Headers/ directory
set(CLIENT_HEADERS
    Headers/mainwindow.hpp
    Headers/loginwidget.hpp
    Headers/services/apiclient.hpp
    Headers/services/authmanager.hpp
    Headers/services/serverconfig.hpp
    Headers/ws/websocket_client.hpp
    Headers/utils/message_json.hpp
    Headers/utils/message_qt_types.hpp
    Headers/models/messagemodel.hpp
    Headers/models/channelmodel.hpp
    Headers/widgets/messageitemwidget.hpp
    Headers/widgets/channelrowwidget.hpp
    Headers/widgets/channelpanelwidget.hpp
    Headers/widgets/rightpanelwidget.hpp
)

# Collect UI files from Forms/ directory
set(CLIENT_UI_FILES
    Forms/mainwindow.ui
    Forms/loginwidget.ui
)

# Collect resource files (.qrc for images, icons, etc.)
set(CLIENT_RESOURCES
    # resources.qrc
)

# Create executable
if(WIN32)
    add_executable(wizzmania-client WIN32 
        ${CLIENT_SOURCES} 
        ${CLIENT_HEADERS}
        ${CLIENT_UI_FILES}
        ${CLIENT_RESOURCES}
    )
else()
    add_executable(wizzmania-client 
        ${CLIENT_SOURCES} 
        ${CLIENT_HEADERS}
        ${CLIENT_UI_FILES}
        ${CLIENT_RESOURCES}
    )
endif()

# Include directories
target_include_directories(wizzmania-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers/services
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers/ws
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers/models
    ${CMAKE_CURRENT_SOURCE_DIR}/Headers/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources/services
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources/ws
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources/models
    ${CMAKE_CURRENT_SOURCE_DIR}/Sources/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/Forms
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_BINARY_DIR}  # For generated UI headers (ui_*.h)
)

# Link Qt libraries
target_link_libraries(wizzmania-client PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebSockets
)

# Windows-specific settings
if(WIN32)
    set_target_properties(wizzmania-client PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # MinGW specific: ensure proper entry point linkage
    if(MINGW)
        target_link_options(wizzmania-client PRIVATE -municode)
    endif()
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(wizzmania-client PRIVATE /W4)
elseif(MINGW OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(wizzmania-client PRIVATE -Wall -Wextra)
endif()

# Deploy Qt DLLs automatically on Windows
if(WIN32)
    # Find windeployqt
    get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET wizzmania-client POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                --no-compiler-runtime
                "$<TARGET_FILE:wizzmania-client>"
            COMMENT "Deploying Qt dependencies..."
        )
    else()
        message(WARNING "windeployqt not found. You may need to manually copy Qt DLLs")
    endif()
endif()