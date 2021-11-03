/*****************************************************************//**
 * \file   JumpGenerater.h
 * \brief  Generate Random 1-2 Jumps
 * 
 * \author peterwhli
 * \date   September 2021
 *********************************************************************/
#pragma once

#include "OsuParser.hpp"
#include "RandomEngine.hpp"
#include <variant>

class JumpGenerator
{
public:
	JumpGenerator(OsuFile& file) : osuFile{ file } {}
	void generate(int count);

	JumpGenerator& setDistance(int distance);
	JumpGenerator& setDistance(int min, int max);
	JumpGenerator& setDistance(Range<int> range);

	~JumpGenerator();
	
private:
	OsuFile& osuFile;

	std::variant<int, Range<int>> length;

	static Coord getNextNotePos(Coord note, float angle, int distance);
};
