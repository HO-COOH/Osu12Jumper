function(AddTest File)
    add_executable("Test.${File}" "${File}.cpp")
    target_link_libraries("Test.${File}" PRIVATE GTest::gtest GTest::gtest_main)
    gtest_discover_tests("Test.${File}")
endfunction()

include(GoogleTest) 

AddTest(StdParse)
AddTest(ManiaParse)
AddTest(Utils)
AddTest(HelperFunction)

add_executable("Test.StdConvertToMania" "StdConvertToMania.cpp" ../BeatmapConvert/BeatmapConvert.cpp)
target_link_libraries("Test.StdConvertToMania" PRIVATE GTest::gtest GTest::gtest_main)
gtest_discover_tests("Test.StdConvertToMania")