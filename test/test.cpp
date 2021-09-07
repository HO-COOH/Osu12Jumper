#include <gtest/gtest.h>
#include "../OsuParser.hpp"
#include <array>

TEST(test, test)
{
	EXPECT_EQ(1, 1);
}

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
	auto [key, value] = SplitKeyVal("key: value");
	EXPECT_EQ(key, "key");
	EXPECT_EQ(value, "value");
}

TEST(Helper, SplitKeyValueNoSpace)
{
	auto [key, value] = SplitKeyVal("key:value");
	EXPECT_EQ(key, "key");
	EXPECT_EQ(value, "value");
}

TEST(Helper, SplitKeyValueEmpty)
{
	auto [key, value] = SplitKeyVal("");
	EXPECT_EQ(key, "");
	EXPECT_EQ(value, "");
}

TEST(Helper, SplitKeyEmpty)
{
	auto [key, value] = SplitKeyVal(":value");
	EXPECT_EQ(key, "");
	EXPECT_EQ(value, "value");
}

TEST(Helper, SplitValueEmpty)
{
	auto [key, value] = SplitKeyVal("key:");
	EXPECT_EQ(key, "key");
	EXPECT_EQ(value, "");
}

TEST(Helper, SplitSpacedStrings)
{
	auto const result = SplitString("hello world you are");
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
	auto const result = SplitString<int>("1 2 3 4 5");
	CompareContainer(
		result,
		std::array{1, 2, 3, 4, 5}
	);
}

TEST(Helper, SplitSpacedStringsSingle)
{
	auto const result = SplitString("hello");
	CompareContainer(result, std::array{ "hello" });
}

TEST(Helper, SplitSpacedStringsEmpty)
{
	auto const result = SplitString("");
	CompareContainer(result, std::array<std::string, 0>{});
}

TEST(Helper, SplitCommaSeperatedStrings)
{
	auto const result = SplitCommaSeperatedString("1,2,3,4,5");
	CompareContainer(
		result,
		std::array{ 1,2,3,4,5 }
	);
}

TEST(ThreeMagic, General)
{
	General const g{ std::ifstream{""} };
	
	EXPECT_EQ(g.audioFile, "3L - Three Magic.mp3");
	EXPECT_EQ(g.audioLeanIn, 0);
	EXPECT_EQ(g.previewTime, 62917);
	EXPECT_EQ(g.countdown, 0);
	EXPECT_EQ(g.sampleSet, SampleSet::Soft);
	EXPECT_EQ(g.stackLeniency, 0.7f);
	EXPECT_EQ(g.mode, Mode::Osu);
	EXPECT_EQ(g.letterboxInBreaks, true);
}

TEST(ThreeMagic, Difficulty)
{
	Difficulty const d{ std::ifstream{""} };

	EXPECT_FLOAT_EQ(d.HPDrainRate, 7.0f);
	EXPECT_FLOAT_EQ(d.circleSize, 4.0f);
	EXPECT_FLOAT_EQ(d.overallDifficulty, 7.0f);
	EXPECT_FLOAT_EQ(d.approachRate, 8.0f);
	EXPECT_FLOAT_EQ(d.sliderMultiplier, 1.8f);
	EXPECT_EQ(d.sliderTickRate, 1);
}

TEST(ThreeMagic, Meta)
{
	Metadata m{ std::ifstream{""} };
	
	EXPECT_EQ(m.title, "Three Magic");
	EXPECT_EQ(m.titleUnicode, "Three Magic");
	EXPECT_EQ(m.artist, "3L");
	EXPECT_EQ(m.artistUnicode, "3L");
	EXPECT_EQ(m.creator, "cRyo[iceeicee]");
	EXPECT_EQ(m.version, "Collab");
	EXPECT_EQ(m.beatmapId, 178643);
	EXPECT_EQ(m.beatmapSetId, 59631);
	
	constexpr std::array tags{
		"mythol",
		"xsrsbsns",
		"clivery",
		"kanpakyin",
		"aabc271",
		"AIRMAN",
		"Kite",
		"ryuu",
		"angel",
		"time",
		"digital",
		"wing",
		"emotional",
		"skyscraper",
		"cosmic",
		"mind",
		"byakuren",
		"hijiri"
	};

	CompareContainer(m.tags, tags);
}

TEST(ThreeMagic, Editor)
{
	Editor e{ std::ifstream{""} };

	EXPECT_FLOAT_EQ(e.distanceSpacing, 0.7f);
	EXPECT_FLOAT_EQ(e.beatDivisor, 0.7f);
	EXPECT_EQ(e.gridSize, 4);
}

TEST(ThreeMagic, HitObject)
{

}
