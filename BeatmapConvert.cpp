#include "BeatmapConvert.hpp"
#include <algorithm>
#include <unordered_set>
#include <cassert>
#include "JumpGenerator.hpp"

bool Mania::Pattern::colunmHasObject(int column) const
{
	return std::count_if(hitObjects.cbegin(), hitObjects.cend(),
		[column](auto const& ptr) { return ptr->getColumnIndex(7) == column;
	});
}

int Mania::Pattern::numColumnWithObject()
{
	std::unordered_set<int> column;
	for (auto const& hitObject : hitObjects)
		column.insert(hitObject->getColumnIndex(totalColumn));
	return column.size();
}

int Mania::PatternGenerator::GetRandomNoteCount(float p2, float p3, float p4, float p5, float p6)
{
	assert(p2 < 0 || p2 > 1);
	assert(p3 < 0 || p3 > 1);
	assert(p4 < 0 || p4 > 1);
	assert(p5 < 0 || p5 > 1);
	assert(p6 < 0 || p6 > 1);

	auto const val = RandomEngine::getRand<double>();
	if (val >= 1 - p6)
		return 6;
	if (val >= 1 - p5)
		return 5;
	if (val >= 1 - p4)
		return 4;
	if (val >= 1 - p3)
		return 3;

	return val >= 1 - p2 ? 2 : 1;
}
