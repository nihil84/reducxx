cmake_minimum_required(VERSION 3.8.0)
project(ReduCppPack VERSION 0.1.1 LANGUAGES CXX)

add_subdirectory(main)
add_subdirectory(test)

if(MSVC) # obviously MSVC is not supported out of the box
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# COVERAGE
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        message("building with coverage data (clang)")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate)
        #Uncomment in case of linker errors
        #link_libraries(clang_rt.profile-x86_64)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message("building with coverage data (gcc)")
        add_compile_options(--coverage)
        #Uncomment in case of linker errors
        #link_libraries(gcov)
    endif ()
endif ()

