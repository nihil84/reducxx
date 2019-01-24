cmake_minimum_required(VERSION 3.0.0)
project(reducpp VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(reducpp src/main.cpp)
set_target_properties(reducpp PROPERTIES
    CXX_STANDARD 17
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
