cmake_minimum_required(VERSION 3.8.0)
project(reducpp VERSION 0.1.1)

include_directories(src)

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
        test/test.cpp
        test/reducpp/basics.cpp
        test/reducpp/composite_state.cpp
        test/reducpp/copy_mng.cpp
        test/reducpp/subscription.cpp
        test/reducpp/concurrency.cpp
        test/reducpp/vs_type_binding.cpp)

set_target_properties(ReduCppTest PROPERTIES CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC) # obviously MSVC is not supported out of the box
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
endif(MSVC)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# COVERAGE
include(code-coverage.cmake)

target_code_coverage(ReduCppTest EXCLUDE test/*)
