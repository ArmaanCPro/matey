cmake_minimum_required(VERSION 3.28)

project (m6502Emu)

set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DM6502_DEBUG" )	#so we can add the DEBUG preprocessor define and other flags to stay in debug mode - see https://cmake.org/Wiki/CMake_Useful_Variables#Compilers_and_Tools
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -DM6502_DEBUG" )

if(MSVC)
    add_compile_options(/MP)				#Use multiple processors when building
    #add_compile_options(/W4 /wd4201 /WX)	#Warning level 4, all warnings are errors
else()
    #add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors
endif()

# google test stuff
include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# source for the test executable
set  (M6502_SOURCES
        "src/main_6502.cpp"
        "src/6502Tests.cpp")

source_group("src" FILES ${M6502_SOURCES})

add_executable( m6502Test ${M6502_SOURCES} 	)
add_dependencies( m6502Test m6502Lib )
target_link_libraries(m6502Test gtest_main)
target_link_libraries(m6502Test m6502Lib)

# Now simply link against gtest or gtest_main as needed. Eg
#add_executable(example example.cpp)
#target_link_libraries(example gtest_main)
#add_test(NAME example_test COMMAND example)