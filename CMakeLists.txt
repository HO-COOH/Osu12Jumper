cmake_minimum_required(VERSION 3.10)

project(OsuJump)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Main main.cpp "JumpGenerator.cpp" "BeatmapConvert.cpp")

#Add testing if found gtest
find_package(GTest CONFIG REQUIRED)

set(OsuTestFileDir "./test")

function(CopyOsuTestFile fileName)
    file(COPY "${OsuTestFileDir}/${fileName}" DESTINATION ${CMAKE_BINARY_DIR})
    message("Copying ${OsuTestFileDir}/${fileName} -> ${CMAKE_BINARY_DIR}")   
endfunction()

if(GTest_FOUND)
    add_subdirectory(${OsuTestFileDir})
    CopyOsuTestFile(TestMapv11.osu)
    CopyOsuTestFile(TestMapv14.osu)
    CopyOsuTestFile(TestMania.osu)
endif()

#set(OsuTestFileDir "${CMAKE_CURRENT_DIR}/test")


add_subdirectory(AutoMapper)