cmake_minimum_required(VERSION 3.8.0)
project(reducpp VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(src)

# Prepare doctest for other targets to use
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE test/doctest.h)

add_executable(ReduCppTest
    src/reducpp/action.hpp
    src/reducpp/composer.hpp
    src/reducpp/reducer_traits.hpp
    src/reducpp/store_factory.hpp
    src/reducpp/store_subscriptions_error.hpp
    src/reducpp/store.hpp
    src/reducpp/async/active_object.hpp
    src/reducpp/async/async_store.hpp
    src/reducpp/async/exception_handling_error.hpp
    test/doctest.cpp 
    test/reducpp/basics.cpp
    test/reducpp/composite_state.cpp
    test/reducpp/copy_mng.cpp
    test/reducpp/subscription.cpp
    test/reducpp/concurrency.cpp
)
target_link_libraries(ReduCppTest doctest)
set_target_properties(ReduCppTest PROPERTIES
    CXX_STANDARD 17
)

add_test(NAME Basics            COMMAND ReduCppTest --test-suite-exclude=concurrency)
add_test(NAME Concurrency       COMMAND ReduCppTest --test-suite=concurrency)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# COVERAGE
#include(code-coverage.cmake)

#target_code_coverage(ReduCppTest)
