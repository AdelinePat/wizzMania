cmake_minimum_required(VERSION 3.16)

project(WizzManiaClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

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
    Sources/services/request_service.cpp
    Sources/services/user_controller.cpp
    Sources/utils/path_utils.cpp
    Sources/services/channel_service.cpp 
    Sources/services/invitation_service.cpp
    Sources/ws/websocket_client.cpp
    Sources/utils/message_json.cpp
    Sources/utils/display.cpp
    Sources/models/message_model.cpp
    Sources/models/channel_model.cpp
    Sources/models/incoming_invitation_model.cpp
    Sources/models/outgoing_invitation_model.cpp
    Sources/widgets/login_widget.cpp
    Sources/widgets/register_widget.cpp
    Sources/widgets/message_item_widget.cpp
    Sources/widgets/channel_row_widget.cpp
    Sources/widgets/channel_panel_widget.cpp
    Sources/widgets/right_panel_widget.cpp
    Sources/widgets/user_home_widget.cpp
    Sources/widgets/incoming_invitation_item_widget.cpp
    Sources/widgets/outgoing_invitation_item_widget.cpp
    Sources/widgets/create_channel_widget.cpp
    ../common/utils.cpp
)

# Collect header files from Headers/ directory
set(CLIENT_HEADERS
    Headers/mainwindow.hpp
    Headers/services/request_service.hpp
    Headers/services/user_controller.hpp
    Headers/utils/path_utils.hpp
    Headers/services/channel_service.hpp  
    Headers/services/invitation_service.hpp
    Headers/ws/websocket_client.hpp
    Headers/utils/message_json.hpp
    Headers/utils/message_qt_types.hpp
    Headers/utils/display.hpp
    Headers/models/message_model.hpp
    Headers/models/channel_model.hpp
    Headers/models/incoming_invitation_model.hpp
    Headers/models/outgoing_invitation_model.hpp
    Headers/widgets/login_widget.hpp
    Headers/widgets/register_widget.hpp
    Headers/widgets/message_item_widget.hpp
    Headers/widgets/channel_row_widget.hpp
    Headers/widgets/channel_panel_widget.hpp
    Headers/widgets/right_panel_widget.hpp
    Headers/widgets/user_home_widget.hpp
    Headers/widgets/incoming_invitation_item_widget.hpp
    Headers/widgets/outgoing_invitation_item_widget.hpp
    Headers/widgets/create_channel_widget.hpp
    ../common/utils.hpp
)

# Collect UI files from Forms/ directory
set(CLIENT_UI_FILES
    Forms/mainwindow.ui
    Forms/login_widget.ui
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
        WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
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