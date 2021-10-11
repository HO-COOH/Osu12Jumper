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
	OsuFile* convertedMap = nullptr;
	void SetUp() override
	{
		static OsuFile f{ std::ifstream{"TestMapV11.osu"} };
		static Mania::ManiaBeatmapConverter converter{ f };
		static auto result = converter.convertBeatmap();
		convertedMap = &result;
	}
};

TEST_F(ManiaConvertFixture, BeatmapInfo)
{

}

TEST_F(ManiaConvertFixture, Columns)
{
	EXPECT_FLOAT_EQ(convertedMap->difficulty.circleSize, 7);
}