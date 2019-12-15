cmake_minimum_required(VERSION 3.8.0)
project(ReduCpp VERSION 0.1.1 LANGUAGES CXX)

add_library(
        ReduCpp INTERFACE
)

target_compile_features(ReduCpp INTERFACE cxx_std_17)
target_include_directories(ReduCpp INTERFACE ./)
