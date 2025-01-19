cmake_minimum_required(VERSION 3.7)

project (m6502Lib)

if (MSVC)
    add_compile_options(/MP)                # multiprocessor compile
    #add_compile_options(/W4 /wd4201 /WX)    # Warning level 4, all warnings are errors

else()
    #add_compile_options(-W -Wall -Werror)
endif()

set( m6502_SOURCES
        "src/6502.h"
        "src/6502.cpp"
)

source_group("src" FILES ${M6502_SOURCES})

# Create the library.
add_library(m6502Lib ${m6502_SOURCES})

# Include the 'src' directory.
target_include_directories(m6502Lib PUBLIC "${PROJECT_SOURCE_DIR}/src")