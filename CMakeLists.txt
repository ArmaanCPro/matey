cmake_minimum_required(VERSION 3.28)

project(matey)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(m6502Lib)
add_subdirectory(m6502Test)