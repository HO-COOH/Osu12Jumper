/*****************************************************************//**
 * \file   HelperFunction.cpp
 * \brief  Test cases for helper functions in the `details::` namespace
 * 
 * \author peterwhli
 * \date   October 2021
 *********************************************************************/
#include "OsuParser.hpp"
#include <gtest/gtest.h>

template<typename Actual, typename Expected>
void CompareContainer(Actual const& actual, Expected const& expected)
{
	ASSERT_EQ(actual.size(), expected.size());
	auto actualIter = std::cbegin(actual);
	for (auto const& expectedItem : expected)
	{
		EXPECT_EQ(expectedItem, *actualIter++);
	}
}

TEST(Helper, SplitKeyValueNormal)
{
	auto [key, value] = details::SplitKeyVal("key: value");
	EXPECT_EQ(key, "key");
	EXPECT_EQ(value, "value");
}

TEST(Helper, SplitKeyValueBothSpaces)
{
	auto [key, value] = details::SplitKeyVal("key : value");
	EXPECT_EQ(key, "key");
	EXPECT_EQ(value, "value");
}

TEST(Helper, SplitKeyValueNoSpace)
{
	auto [key, value] = details::SplitKeyVal("key:value");
	EXPECT_EQ(key, "key");
	EXPECT_EQ(value, "value");
}

TEST(Helper, SplitKeyValueEmpty)
{
	auto [key, value] = details::SplitKeyVal("");
	EXPECT_EQ(key, "");
	EXPECT_EQ(value, "");
}

TEST(Helper, SplitKeyEmpty)
{
	auto [key, value] = details::SplitKeyVal(":value");
	EXPECT_EQ(key, "");
	EXPECT_EQ(value, "value");
}

TEST(Helper, SplitValueEmpty)
{
	auto [key, value] = details::SplitKeyVal("key:");
	EXPECT_EQ(key, "key");
	EXPECT_EQ(value, "");
}

TEST(Helper, SplitSpacedStrings)
{
	auto const result = details::SplitWords("hello world you are");
	CompareContainer(
		result,
		std::array{
			"hello",
			"world",
			"you",
			"are"
		}
	);
}

TEST(Helper, SplitSpacedInts)
{
	auto const result = details::SplitWords<int>("1 2 3 4 5");
	CompareContainer(
		result,
		std::array{ 1, 2, 3, 4, 5 }
	);
}

TEST(Helper, SplitSpacedStringsSingle)
{
	auto const result = details::SplitWords("hello");
	CompareContainer(result, std::array{ "hello" });
}

TEST(Helper, SplitSpacedStringsEmpty)
{
	auto const result = details::SplitWords("");
	CompareContainer(result, std::array<std::string, 0>{});
}

TEST(Helper, SplitCommaSeperatedStrings)
{
	auto const result = details::SplitCommaSeparatedString("1,2,3,4,5");
	CompareContainer(
		result,
		std::array{ 1,2,3,4,5 }
	);
}

TEST(Helper, SplitFixed)
{
	auto const result = details::SplitString<3>("1,2,3");
	CompareContainer(
		std::array{
			std::stoi(result[0].data()),
			std::stoi(result[1].data()),
			std::stoi(result[2].data())
		},
		std::array{ 1,2,3 }
	);
}

TEST(Helper, SplitFixedColon)
{
	auto const result = details::SplitString<5>("0:1:2:3:", ':');
	CompareContainer(
		std::array{
			std::stoi(result[0].data()),
			std::stoi(result[1].data()),
			std::stoi(result[2].data()),
			std::stoi(result[3].data()),
			result[4].empty() ? 0 : std::stoi(result[4].data()),
		},
		std::array{ 0,1,2,3,0 }
	);
}

TEST(Helper, SplitCurvePoints)
{
	auto const result = details::SplitString("120:272|80:240", '|');
	CompareContainer(
		result,
		std::array{ "120:272", "80:240" }
	);
}

TEST(Helper, SplitEvent)
{
	auto const result = details::SplitString<3>(R"(Sprite,Foreground,Centre,"neimuu\Mythol.png",320,240)", ',', true);
	CompareContainer(
		result,
		std::array{
			"Sprite",
			"Foreground",
			R"(Centre,"neimuu\Mythol.png",320,240)"
		}
	);
}
TEST(Helper, SplitOptionalSliderParam)
{
	auto const result = details::SplitString<11>("404,120,44741,2,0,B|420:16,1,90");
	CompareContainer(
		result,
		std::array{
			"404",
			"120",
			"44741",
			"2",
			"0",
			"B|420:16",
			"1",
			"90",
			"",
			"",
			""
		}
	);
}

#include "RandomEngine.hpp"
TEST(Random, RandomFloat)
{
	auto const rand = RandomEngine::getRand();
	EXPECT_TRUE(0.0 <= rand && rand <= 1.0);
}

TEST(Random, RandomInt)
{
	auto const rand = RandomEngine::getRand(Range{ 1, 7 });
	EXPECT_TRUE(1 <= rand && rand <= 7);
}
