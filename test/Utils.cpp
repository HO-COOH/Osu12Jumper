/*****************************************************************//**
 * \file   Utils.cpp
 * \brief  Test cases for utility functions for getting infos from osu beatmaps
 * 
 * \author peterwhli
 * \date   October 2021
 *********************************************************************/

#include "OsuParser.hpp"
#include <gtest/gtest.h>

constexpr std::string_view fileName = "3L - Three Magic (cRyo[iceeicee]) [ryuu's Easy].osu";
TEST(ParseFileName, Artist)
{
	EXPECT_EQ(OsuFile::ParseArtistFrom(fileName), "3L");
}

TEST(ParseFileName, Title)
{
	EXPECT_EQ(OsuFile::ParseTitleFrom(fileName), "Three Magic");
}

TEST(ParseFileName, Creator)
{
	EXPECT_EQ(OsuFile::ParseCreatorFrom(fileName), "cRyo");
}

TEST(ParseFileName, Version)
{
	EXPECT_EQ(OsuFile::ParseVersionFrom(fileName), "ryuu's Easy");
}

TEST(SaveFileName, RemoveColon)
{
	EXPECT_EQ(
		details::RemoveInStr("Kirisaki Chitoge (CV: Touyama Nao), Onodera Kosaki (CV: Hanazawa Kana)"),
		std::string{ "Kirisaki Chitoge (CV Touyama Nao), Onodera Kosaki (CV Hanazawa Kana)" }
	);
}
