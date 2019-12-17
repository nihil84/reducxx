cmake_minimum_required(VERSION 3.8.0)
project(ReduCxx VERSION 0.1.1 LANGUAGES CXX)

add_library(
        ReduCxx INTERFACE
)

target_compile_features(ReduCxx INTERFACE cxx_std_17)
target_include_directories(ReduCxx INTERFACE ./)
