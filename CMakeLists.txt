cmake_minimum_required(VERSION 3.16)

project(ContactManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Sql 
    Network
)

# Source files
set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/databasemanager.cpp
    src/databasemanager.h
    src/networkmanager.cpp
    src/networkmanager.h
    src/contact.h
)

# Create executable
add_executable(ContactManager ${PROJECT_SOURCES})

# Link Qt libraries
target_link_libraries(ContactManager PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)

# Include directories
target_include_directories(ContactManager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Platform-specific settings
if(WIN32)
    set_target_properties(ContactManager PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# Install target
install(TARGETS ContactManager
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
