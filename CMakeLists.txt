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

