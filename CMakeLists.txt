cmake_minimum_required(VERSION 3.0.0)
project(reducpp VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(src)

add_executable(doctest 
    test/doctest.cpp 
    test/reducpp/basics.cpp
    test/reducpp/composite_state.cpp
    test/reducpp/copy_mng.cpp
    test/reducpp/subscription.cpp
    test/reducpp/concurrency.cpp
)
add_test(NAME ReduCppTest COMMAND doctest)
set_target_properties(doctest PROPERTIES
    CXX_STANDARD 17
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
