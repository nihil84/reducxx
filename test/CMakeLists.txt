

add_executable(
        ReduCppTest
        test.cpp
        reducpp/basics.cpp
        reducpp/composite_state.cpp
        reducpp/copy_mng.cpp
        reducpp/subscription.cpp
        reducpp/concurrency.cpp
        reducpp/vs_type_binding.cpp
)

target_compile_features(ReduCppTest PRIVATE cxx_std_17)

target_link_libraries(
        ReduCppTest
        PRIVATE ReduCpp
)


option(ENABLE_COVERAGE "when on coverage data is added in debug builds" ON)

# COVERAGE
if (ENABLE_COVERAGE)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        message("building with coverage data (clang)")
        target_compile_options(ReduCppTest PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(ReduCppTest PRIVATE -fprofile-instr-generate)
        #Uncomment in case of linker errors
        #target_link_libraries(ReduCppTest PRIVATE clang_rt.profile-x86_64)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message("building with coverage data (gcc)")
        target_compile_options(ReduCppTest PRIVATE --coverage)
        #Uncomment in case of linker errors
        target_link_libraries(ReduCppTest PRIVATE gcov)
    endif ()
endif ()
