cmake_minimum_required(VERSION 3.10)
project(CakemuOoO VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find SystemC package
find_package(SystemCLanguage CONFIG REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Create executable
add_executable(cakemu_ooo ${SOURCES})
target_link_libraries(cakemu_ooo SystemC::systemc)

# Install
install(TARGETS cakemu_ooo DESTINATION bin)
