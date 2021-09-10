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
		std::array{1, 2, 3, 4, 5}
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
	auto const result = details::SplitCommaSeperatedString("1,2,3,4,5");
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
			result[4].empty()? 0 : std::stoi(result[4].data()),
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

class Fixture
{
protected:
	General const* general;
	Difficulty const* difficulty;
	Metadata const* metadata;
	Editor const* editor;
	std::vector<Event> const* events;
	std::vector<TimingPoint> const* timingPoints;
	Colors const* colors;
	std::vector<std::unique_ptr<HitObject>> const* hitObjects;
	OsuFile* file;

	void SetMember(OsuFile& f)
	{
		general = &f.general;
		difficulty = &f.difficulty;
		metadata = &f.metaData;
		editor = &f.editor;
		events = &f.events;
		timingPoints = &f.timingPoints;
		colors = &f.colors;
		hitObjects = &f.hitObjects;
		file = &f;
	}
};

class ParseV11 : public ::testing::Test, public Fixture
{

protected:
	void SetUp() override
	{
		static OsuFile f{ std::ifstream{ "TestMapV11.osu"} };
		SetMember(f);
	}
};

class ParseV14 : public ::testing::Test, public Fixture
{
protected:
	void SetUp() override
	{
		static OsuFile f{ std::ifstream{ "TestMapV14.osu"} };
		SetMember(f);
	}
};

TEST_F(ParseV11, General)
{
	General const& g = *general;
	
	EXPECT_EQ(g.audioFile, "3L - Three Magic.mp3");
	EXPECT_EQ(g.audioLeanIn, 0);
	EXPECT_EQ(g.previewTime, 62917);
	EXPECT_EQ(g.countdown, Countdown::No);
	EXPECT_EQ(g.sampleSet, SampleSet::Soft);
	EXPECT_FLOAT_EQ(g.stackLeniency, 0.7f);
	EXPECT_EQ(g.mode, Mode::Osu);
	EXPECT_EQ(g.letterboxInBreaks, true);
}

TEST_F(ParseV14, General)
{
	General const& g = *general;

	EXPECT_EQ(g.audioFile, "audio.mp3");
	EXPECT_EQ(g.audioLeanIn, 0);
	EXPECT_EQ(g.previewTime, 65575);
	EXPECT_EQ(g.countdown, Countdown::No);
	EXPECT_EQ(g.sampleSet, SampleSet::Soft);
	EXPECT_FLOAT_EQ(g.stackLeniency, 0.7f);
	EXPECT_EQ(g.mode, Mode::Osu);
	EXPECT_FALSE(g.letterboxInBreaks);
	EXPECT_FALSE(g.wideScreenStoryboard);
}

TEST_F(ParseV11, Editor)
{
	Editor const& e = *editor;

	EXPECT_FLOAT_EQ(e.distanceSpacing, 0.7f);
	EXPECT_EQ(e.beatDivisor, 4);
	EXPECT_EQ(e.gridSize, 4);
}

TEST_F(ParseV14, Editor)
{
	Editor const& e = *editor;

	EXPECT_FLOAT_EQ(e.distanceSpacing, 1.8f);
	EXPECT_EQ(e.beatDivisor, 4);
	EXPECT_EQ(e.gridSize, 4);
	EXPECT_FLOAT_EQ(e.timelineZoom, 2.5f);
}

TEST_F(ParseV11, Meta)
{
	Metadata const& m = *metadata;

	EXPECT_EQ(m.title, "Three Magic");
	EXPECT_EQ(m.titleUnicode, u8"Three Magic");
	EXPECT_EQ(m.artist, "3L");
	EXPECT_EQ(m.artistUnicode, u8"3L");
	EXPECT_EQ(m.creator, "cRyo[iceeicee]");
	EXPECT_EQ(m.version, "Collab");
	EXPECT_EQ(m.beatmapId, 178643);
	EXPECT_EQ(m.beatmapSetId, 59631);

	CompareContainer(
		m.tags,
		std::array{
			"mythol",
			"xsrsbsns",
			"cilvery",
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
		}
	);
}

TEST_F(ParseV14, Meta)
{
	Metadata const& m = *metadata;

	EXPECT_EQ(m.title, "Colorful");
	EXPECT_EQ(m.titleUnicode, u8"����ե�");
	EXPECT_EQ(m.artist, "HAG");
	EXPECT_EQ(m.artistUnicode, u8"H��G");
	EXPECT_EQ(m.creator, "Sotarks");
	EXPECT_EQ(m.version, "Prismatic");
	EXPECT_EQ(m.source, "");
	EXPECT_EQ(m.beatmapId, 1944926);
	EXPECT_EQ(m.beatmapSetId, 931452);

	CompareContainer(
		m.tags,
		std::array{
			"akitoshi",
			"fieryrage",
			"corinn",
			"-laura-",
			"nikakis",
			"snownino_",
			"miraclee",
			"nevo",
			"reform",
			"kujinn",
			"armin",
			"a_r_m_i_n", 
			"smokelind",
			"onlybiscuit",
			"brotarks",
			"lami",
			"taeyang",
			"kroytz",
			"japanese",
			"female",
			"vocalist",
			"jpop",
			"j-pop",
			"cut",
			"edit",
			"short",
			"version",
			"everlasting",
			"night",
			"of",
			"teenage",
			"girls",
			"ayyri",
			"eiri-",
			"colourful"
		}
	);
}


TEST_F(ParseV11, Difficulty)
{
	Difficulty const& d = *difficulty;

	EXPECT_FLOAT_EQ(d.HPDrainRate, 7.0f);
	EXPECT_FLOAT_EQ(d.circleSize, 4.0f);
	EXPECT_FLOAT_EQ(d.overallDifficulty, 7.0f);
	EXPECT_FLOAT_EQ(d.approachRate, 8.0f);
	EXPECT_FLOAT_EQ(d.sliderMultiplier, 1.8f);
	EXPECT_EQ(d.sliderTickRate, 1);
}

TEST_F(ParseV14, Difficulty)
{
	Difficulty const& d = *difficulty;

	EXPECT_FLOAT_EQ(d.HPDrainRate, 5.2f);
	EXPECT_FLOAT_EQ(d.circleSize, 4);
	EXPECT_FLOAT_EQ(d.overallDifficulty, 9.4f);
	EXPECT_FLOAT_EQ(d.approachRate, 9.4f);
	EXPECT_FLOAT_EQ(d.sliderMultiplier, 1.92f);
	EXPECT_EQ(d.sliderTickRate, 1);
}


TEST_F(ParseV11, Events)
{

}

TEST_F(ParseV14, Events)
{

}

static inline void Compare(TimingPoint const& lhs, TimingPoint const& rhs)
{
	EXPECT_EQ(lhs.time, rhs.time);
	EXPECT_FLOAT_EQ(lhs.beatLength, rhs.beatLength);
	EXPECT_EQ(lhs.meter, rhs.meter);
	EXPECT_EQ(lhs.sampleSet, rhs.sampleSet);
	EXPECT_EQ(lhs.sampleIndex, rhs.sampleIndex);
	EXPECT_EQ(lhs.volume, rhs.volume);
	EXPECT_EQ(lhs.uninherited, rhs.uninherited);
	EXPECT_EQ(lhs.effects, rhs.effects);
}

TEST_F(ParseV11, TimingPoints)
{

}

TEST_F(ParseV14, TimingPoints)
{
	auto const& t = *timingPoints;

	EXPECT_EQ(t.size(), 23);

	TimingPoint first;
	first.time = 620;
	first.beatLength = 312.5;
	first.meter = 4;
	first.sampleSet = SampleSet::Soft;
	first.sampleIndex = 22;
	first.volume = 42;
	first.uninherited = 1;
	first.effects = 0;
	Compare(t.front(), first);

	TimingPoint last;
	last.time = 135620;
	last.beatLength = -833.3333333333f;
	last.meter = 4;
	last.sampleSet = SampleSet::Soft;
	last.sampleIndex = 22;
	last.volume = 12;
	last.uninherited = 0;
	last.effects = 0;
	Compare(t.back(), last);
}

TEST_F(ParseV11, Colors)
{
	Colors const& c = *colors;
	CompareContainer(
		c.comboColor,
		std::array{ 
			Colors::Color(255, 255, 128),
			Colors::Color(255, 128, 192),
			Colors::Color(146, 146, 201)
		}
	);
}


TEST_F(ParseV11, HitObject)
{
	EXPECT_EQ(file->getCount<HitObject::Type::Circle>(), 364);
	EXPECT_EQ(file->getCount<HitObject::Type::Slider>(), 228);
	EXPECT_EQ(file->getCount<HitObject::Type::Spinner>(), 2);
	EXPECT_EQ(file->getCount(), 364 + 228 + 2);

	auto firstSliderIter = file->begin<HitObject::Type::Slider>();
	firstSliderIter->x = 1;
	auto firstAllIter = file->begin();
	EXPECT_EQ(firstAllIter->x, 1);
}

TEST_F(ParseV14, HitObject)
{
	EXPECT_EQ(file->getCount<HitObject::Type::Circle>(), 373);
	EXPECT_EQ(file->getCount<HitObject::Type::Slider>(), 178);
	EXPECT_EQ(file->getCount<HitObject::Type::Spinner>(), 0);
	EXPECT_EQ(file->getCount(), 373 + 178 + 0);
}