cmake_minimum_required(VERSION 3.16)
project(ncmtool)

set(CMAKE_CXX_STANDARD 17)

# Path to Qt installation
# Modify this path if Qt is installed in a different location
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/msvc2022_64/lib/cmake")

# Qt6 modules
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# Enable Qt features
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Include directories for headers
include_directories(
    src/ui
    src/core
)

# Executable source files
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.ui
    src/core/Worker.h
    src/core/Worker.cpp
    res/app_icon.rc
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

# Set Windows GUI app flag (no console)
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)