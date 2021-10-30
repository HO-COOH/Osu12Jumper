#include "include/PatternGenerator.hpp"
#include "RandomEngine.hpp"
#include <cassert>
#include <optional>

int Mania::PatternGenerator::GetRandomNoteCount(double p2, double p3, double p4, double p5, double p6)
{
    assert(p2 >= 0 && p2 <= 1);
    assert(p3 >= 0 && p3 <= 1);
    assert(p4 >= 0 && p4 <= 1);
    assert(p5 >= 0 && p5 <= 1);
    assert(p6 >= 0 && p6 <= 1);

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

int Mania::PatternGenerator::getRandomColumn(std::optional<int> lowerBound, std::optional<int> upperBound) const
{
    auto const low = lowerBound.value_or(randomStart);
    auto const high = upperBound.value_or(totalColumns);
    return RandomEngine::getRand(low, high);
}

int Mania::PatternGenerator::getRandomColumn() const
{
    return RandomEngine::getRand(randomStart, totalColumns - 1);
}

int Mania::PatternGenerator::findAvailableColumn(
    int initialColumn,
    std::optional<int> lowerBound,
    std::optional<int> upperBound,
    std::function<int(int)> nextColumn,
    std::function<bool(int)> validator,
    std::vector<Pattern const*> patterns
) const
{
    auto isValid = [&validator, &patterns](int column)
    {
        auto const noOccupy = !std::any_of(patterns.cbegin(), patterns.cend(), [column](Pattern const* p) { return p->colunmHasObject(column); });
        return validator ? validator(column) && noOccupy : noOccupy;
    };

    auto const lowBound = lowerBound.value_or(randomStart);
    auto const upBound = upperBound.value_or(previousPattern.totalColumn - 1);
    assert(lowBound >= 0);
    assert(upBound < totalColumns);
    assert(lowBound <= upBound);

    if (isValid(initialColumn))
        return initialColumn;

    // Ensure that we have at least one free column, so that an endless loop is avoided
    bool hasValidColumns = false;
    for (int i = lowBound; i <= upBound; ++i)
    {
        hasValidColumns = isValid(i);
        if (hasValidColumns)
            break;
    }

    assert(hasValidColumns); //This should almost never happens?
    if (!hasValidColumns)
        return -1;

    // Iterate until a valid column is found. This is a random iteration in the default case.
    do
    {
        initialColumn = nextColumn ? nextColumn(initialColumn) : getRandomColumn(lowBound, upBound);
    } while (!isValid(initialColumn));

    assert(initialColumn >= 0 && initialColumn < totalColumns);
    return initialColumn;
}

int Mania::PatternGenerator::findAvailableColumn(int initialColumn, std::vector<Pattern const*> patterns) const
{
    return findAvailableColumn(initialColumn, {}, {}, {}, {}, patterns);
}

double Mania::PatternGenerator::getConversionDifficulty()
{
    if (conversionDifficulty.has_value())
        return *conversionDifficulty;

    auto drainTimeInSec = originalBeatmap.getDrainTime() / 1'000;
    if (drainTimeInSec == 0)
        drainTimeInSec = 10'000;

    Difficulty const& baseDifficulty = originalBeatmap.difficulty;
    double value = ((baseDifficulty.HPDrainRate + std::clamp(baseDifficulty.approachRate, 4.f, 7.f)) / 1.5 + originalBeatmap.getCount() / drainTimeInSec * 9.f) / 38.f * 5.f / 1.15;
    value = std::min(value, 12.0);
    conversionDifficulty = value;
    return value;
}
