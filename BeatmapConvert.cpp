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

int Mania::PatternGenerator::getRandomNoteCount(double p2, double p3, double p4, double p5) const
{
    switch (previousPattern.totalColumn)
    {
        case 2:
            p2 = 0;
            p3 = 0;
            p4 = 0;
            p5 = 0;
            break;

        case 3:
            p2 = std::min(p2, 0.1);
            p3 = 0;
            p4 = 0;
            p5 = 0;
            break;

        case 4:
            p2 = std::min(p2, 0.23);
            p3 = std::min(p3, 0.04);
            p4 = 0;
            p5 = 0;
            break;

        case 5:
            p3 = std::min(p3, 0.15);
            p4 = std::min(p4, 0.03);
            p5 = 0;
            break;
    }

    if (hitObject->hitSample.has(HitObject::HitSound::Clap))
        p2 = 1;

    return GetRandomNoteCount(p2, p3, p4, p5);
}

int Mania::PatternGenerator::getRandomNoteMirrored(double centreProbability, double p2, double p3, bool& addToCentre)
{
    auto const column = previousPattern.totalColumn;
    switch (column)
    {
    case 2:
        centreProbability = 0;
        p2 = 0;
        p3 = 0;
        break;

    case 3:
        centreProbability = std::min(centreProbability, 0.03);
        p2 = 0;
        p3 = 0;
        break;

    case 4:
        centreProbability = 0;

        // Stable requires rngValue > x, which is an inverse-probability. Lazer uses true probability (1 - x).
        // But multiplying this value by 2 (stable) is not the same operation as dividing it by 2 (lazer),
        // so it needs to be converted to from a probability and then back after the multiplication.
        p2 = 1 - std::max((1 - p2) * 2, 0.8);
        p3 = 0;
        break;

    case 5:
        centreProbability = std::min(centreProbability, 0.03);
        p3 = 0;
        break;

    case 6:
        centreProbability = 0;

        // Stable requires rngValue > x, which is an inverse-probability. Lazer uses true probability (1 - x).
        // But multiplying this value by 2 (stable) is not the same operation as dividing it by 2 (lazer),
        // so it needs to be converted to from a probability and then back after the multiplication.
        p2 = 1 - std::max((1 - p2) * 2, 0.5);
        p3 = 1 - std::max((1 - p3) * 2, 0.85);
        break;
    }

    // The stable values were allowed to exceed 1, which indicate <0% probability.
    // These values needs to be clamped otherwise GetRandomNoteCount() will throw an exception.
    p2 = std::clamp(p2, 0., 1.);
    p3 = std::clamp(p3, 0., 1.);

    auto centreVal = RandomEngine::getRand<double>();
    int noteCount = GetRandomNoteCount(p2, p3);

    addToCentre = column % 2 != 0 && noteCount != 3 && centreVal > 1 - centreProbability;
    return noteCount;
}


int Mania::PatternGenerator::getRandomColumn(std::optional<int> lowerBound, std::optional<int> upperBound) const
{
    auto const low = lowerBound.value_or(randomStart);
    auto const high = upperBound.value_or(previousPattern.totalColumn);
    return RandomEngine::getRand(low, high);
}

int Mania::PatternGenerator::findAvailableColumn(
    int initialColumn,
    std::optional<int> lowerBound,
    std::optional<int> upperBound,
    std::function<int(int, int)> nextColumn,
    std::function<bool(int)> validator
)
{
    auto const lowBound = lowerBound.value_or(randomStart);
    auto const upBound = upperBound.value_or(previousPattern.totalColumn);
    //if (!nextColumn)
    //    nextColumn = std::move([this, lowBound, upBound](int column) { return getRandomColumn(lowBound, upBound); });

    if (validator && validator(initialColumn))
        return initialColumn;

    bool hasValidColumns = false;
    for (int i = 0; i < lowBound; ++i)
    {
        if(validator(i))
        {
            hasValidColumns = true;
            break;
        }
    }

    if (!hasValidColumns)
        return -1;

    do {
        if ();
    } while()

}

double Mania::PatternGenerator::getConversionDifficulty() const
{
    auto const drainTimeInSec = beatmap.getDrainTime() / 1'000;

    Difficulty const& baseDifficulty = beatmap.difficulty;

}

