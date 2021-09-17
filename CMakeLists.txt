cmake_minimum_required(VERSION 3.10)

project(OsuJump)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Main main.cpp "JumpGenerator.cpp")

#Add testing if found gtest
find_package(GTest CONFIG REQUIRED)
if(GTest_FOUND)

    set(TestTarget Test)

    include(GoogleTest) 
    add_executable(${TestTarget} test/test.cpp)
    enable_testing()
    target_link_libraries(${TestTarget} PRIVATE GTest::gtest GTest::gtest_main)
    gtest_discover_tests(${TestTarget}) 
endif()

#set(OsuTestFileDir "${CMAKE_CURRENT_DIR}/test")
set(OsuTestFileDir "./test")
function(CopyOsuTestFile fileName)
    file(COPY "${OsuTestFileDir}/${fileName}" DESTINATION ${CMAKE_BINARY_DIR})
    message("Copying ${OsuTestFileDir}/${fileName} -> ${CMAKE_BINARY_DIR}")   
endfunction()

CopyOsuTestFile(TestMapv11.osu)
CopyOsuTestFile(TestMapv14.osu)

