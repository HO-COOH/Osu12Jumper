#include "BeatmapConvert/include/BeatmapConvert.hpp"
#include <gtest/gtest.h>

TEST(HelperFunction, FlagChange)
{
	Mania::Pattern::Type type{};

	auto& flag = reinterpret_cast<unsigned&>(type);
	flag |= Mania::Pattern::Type::Alternate;
	flag |= Mania::Pattern::Type::Stair;

	EXPECT_TRUE(type & Mania::Pattern::Type::Stair);
	EXPECT_TRUE(type & Mania::Pattern::Type::Alternate);
	EXPECT_FALSE(type & Mania::Pattern::Type::ReverseStair);
}

class ManiaConvertFixture : public ::testing::Test
{
protected:
	OsuFile* originalMap = nullptr;
	OsuFile* convertedMap = nullptr;
	void SetUp() override
	{
		static OsuFile f{ std::ifstream{"TestMapv11.osu"} };
		static Mania::ManiaBeatmapConverter converter{ f };
		static auto result = converter.convertBeatmap();
		convertedMap = &result;
		originalMap = &f;
	}
};

TEST_F(ManiaConvertFixture, BeatmapInfo)
{

}

TEST_F(ManiaConvertFixture, Columns)
{
	EXPECT_FLOAT_EQ(convertedMap->difficulty.circleSize, 7);
}

TEST_F(ManiaConvertFixture, SliderEndTime)
{
	//EXPECT_EQ(convertedMap->hitObjects)

	/*DistanceObject*/
	Mania::DistanceObjectPatternGenerator gen{
		*originalMap->hitObjects[0],
		*convertedMap,
		Mania::Pattern{7},
		*originalMap,
		7
	};



}
