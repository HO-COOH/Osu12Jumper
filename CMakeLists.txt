cmake_minimum_required(VERSION 3.10)

project(OsuJump)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Main main.cpp)

#Add testing if found gtest
find_package(GTest CONFIG REQUIRED)

    set(TestTarget Test)

    include(GoogleTest) 
    add_executable(${TestTarget} test/test.cpp)
    enable_testing()
    target_link_libraries(${TestTarget} PRIVATE GTest::gtest GTest::gtest_main)
    gtest_discover_tests(${TestTarget}) 
