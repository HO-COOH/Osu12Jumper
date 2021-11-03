#include <gtest/gtest.h>
#include "../OsuParser.hpp"

TEST(ManiaSpecificObject, ColumnsToX4K)
{
	Circle const c1{ 64, 192, {}, {}, {} };
	Circle const c2{ 192, 192, {}, {}, {} };
	Circle const c3{ 320, 192, {}, {}, {} };
	Circle const c4{ 448, 192, {}, {}, {} };
	EXPECT_EQ(c1.getColumnIndex(4), 0);
	EXPECT_EQ(c2.getColumnIndex(4), 1);
	EXPECT_EQ(c3.getColumnIndex(4), 2);
	EXPECT_EQ(c4.getColumnIndex(4), 3);
}

TEST(ManiaSpecificObject, ColumnsToX7K)
{
	Circle const c1{ 36, 192, {}, {}, {} };
	Circle const c2{ 109, 192, {}, {}, {} };
	Circle const c3{ 182, 192, {}, {}, {} };
	Circle const c4{ 256, 192, {}, {}, {} };
	Circle const c5{ 329, 192, {}, {}, {} };
	Circle const c6{ 406, 192, {}, {}, {} };
	Circle const c7{ 475, 192, {}, {}, {} };

	EXPECT_EQ(c1.getColumnIndex(7), 0);
	EXPECT_EQ(c2.getColumnIndex(7), 1);
	EXPECT_EQ(c3.getColumnIndex(7), 2);
	EXPECT_EQ(c4.getColumnIndex(7), 3);
	EXPECT_EQ(c5.getColumnIndex(7), 4);
	EXPECT_EQ(c6.getColumnIndex(7), 5);
	EXPECT_EQ(c7.getColumnIndex(7), 6);
}

TEST(ManiaSpecificObject, XToColumn4K)
{
	for (int i = 0; i < 4; ++i)
	{
		auto const actual =  Circle{ HitObject::ColumnToX(i, 4), 192, {}, {}, {} }.getColumnIndex(4);
		EXPECT_EQ(actual, i);
	}
}

TEST(ManiaSpecificObject, XToColumn7K)
{
	for (int i = 0; i < 7; ++i)
	{
		auto const actual = Circle{ HitObject::ColumnToX(i, 7), 192, {}, {}, {} }.getColumnIndex(7);
		EXPECT_EQ(actual, i);
	}
}

class Fixture
{
protected:
	General const* general;
	Difficulty const* difficulty;
	Metadata const* metadata;
	Editor const* editor;
	Events const* events;
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

class ParseMania : public ::testing::Test, public Fixture
{
protected:
	void SetUp() override
	{
		static OsuFile f{ std::ifstream{"TestMania.osu"} };
		SetMember(f);
	}
};

TEST_F(ParseMania, HitObject)
{
	auto const& objs = *hitObjects;
	EXPECT_EQ(objs.size(), 736);
	EXPECT_EQ(file->getCount<HitObject::Type::Circle>(), 632);
	EXPECT_EQ(file->getCount<HitObject::Type::Hold>(), 104);
}

TEST_F(ParseMania, Columns)
{
	EXPECT_EQ(difficulty->circleSize, 4);
}
