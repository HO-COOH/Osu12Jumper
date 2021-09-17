#include <iostream>
#include "JumpGenerator.hpp"

int main()
{
	OsuFile file{ std::ifstream{"TestMapv11.osu"} };
	file.metaData.version = "test";
	JumpGenerator gen{ file };
	gen.setDistance(Range{ 50, 200 }).generate(50);
}