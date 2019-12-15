

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