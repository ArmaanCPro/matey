cmake_minimum_required(VERSION 3.20)

project (m6502)

if (MSVC)
    add_compile_options(/MP)                # multiprocessor compile
    #add_compile_options(/W4 /wd4201 /WX)    # Warning level 4, all warnings are errors

else()
    #add_compile_options(-W -Wall -Werror)
endif()

set( m6502_SOURCES
        src/main.cpp
)

# Create the executable.
add_executable(m6502 ${m6502_SOURCES})

# Include the 'src' directory.
target_include_directories(m6502 PUBLIC "${CMAKE_SOURCE_DIR}/src")