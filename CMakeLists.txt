cmake_minimum_required(VERSION 3.10)

project(OsuJump)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
include_directories(${CMAKE_SOURCE_DIR})

#TODO: Add test map to executable
add_executable(Main main.cpp 
    "JumpGenerator.cpp" 
    "BeatmapConvert/BeatmapConvert.cpp" 
    "BeatmapConvert/PatternGenerator.cpp"
    "BeatmapConvert/Mania.Pattern.cpp"
)
if(UNIX)
    target_link_libraries(Main pthread)
    add_compile_definitions(DEBUG)
endif()

#Add testing if found gtest
find_package(GTest CONFIG)

set(OsuTestFileDir "./test")

function(CopyOsuTestFile fileName)
    file(COPY "${OsuTestFileDir}/${fileName}" DESTINATION ${CMAKE_BINARY_DIR}/test)
    message("Copying ${OsuTestFileDir}/${fileName} -> ${CMAKE_BINARY_DIR}/test")   
endfunction()

if(GTest_FOUND)
    enable_testing()
    add_subdirectory(${OsuTestFileDir})
    CopyOsuTestFile(TestMapv11.osu)
    CopyOsuTestFile(TestMapv14.osu)
    CopyOsuTestFile(TestMania.osu)

    #copy one extra file for testing converted beatmap save path
    file(COPY "${OsuTestFileDir}/TestMapv11.osu" DESTINATION ${CMAKE_BINARY_DIR}/test/test)
endif()

#set(OsuTestFileDir "${CMAKE_CURRENT_DIR}/test")


add_subdirectory(AutoMapper)