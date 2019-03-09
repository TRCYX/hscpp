cmake_minimum_required(VERSION 3.0.0)
project(hscpp VERSION 0.1.0)

include_directories(include/)
add_executable(hs src/hs.cpp)
set_property(TARGET hs PROPERTY CXX_STANDARD 17)
