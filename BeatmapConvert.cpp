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

int Mania::Pattern::numColumnWithObject() const
{
	std::unordered_set<int> column;
	for (auto const& hitObject : hitObjects)
		column.insert(hitObject->getColumnIndex(totalColumn));
	return column.size();
}

Mania::Pattern& Mania::Pattern::operator+=(std::unique_ptr<HitObject> hitObject)
{
    hitObjects.emplace_back(std::move(hitObject));
    return *this;
}

Mania::PatternGenerator::PatternGenerator(std::unique_ptr<HitObject> const& hitObject, OsuFile& beatmap, Pattern const& previousPattern, int previousTime, Coord previousPosition, double density, Pattern::Type lastStair, OsuFile const& originalBeatmap)
    : hitObject{hitObject},
    beatmap{beatmap},
    previousPattern{previousPattern},
    stairType{lastStair},
    originalBeatmap{originalBeatmap}
{
    auto const& timingPoint = originalBeatmap.getTimingPointAt(hitObject->time);
    

    Coord const positionData{ hitObject->x, hitObject->y };
    
    auto const positionSeparation = positionData.distanceTo(previousPosition);
    auto const timeSeparation = hitObject->time - previousTime;

    auto& convertTypeFlag = reinterpret_cast<unsigned&>(convertType);
    
    if (timeSeparation <= 80)       convertTypeFlag |= Pattern::Type::ForceNotStack | Pattern::Type::KeepSingle;
    else if (timeSeparation <= 95)  convertTypeFlag |= Pattern::Type::ForceNotStack | Pattern::Type::KeepSingle | lastStair;
    else if (timeSeparation <= 105) convertTypeFlag |= Pattern::Type::ForceNotStack | Pattern::Type::LowProbability;
    else if (timeSeparation <= 125) convertTypeFlag |= Pattern::Type::ForceNotStack;
    else if (timeSeparation <= 135 && positionSeparation < 20) convertTypeFlag |= Pattern::Type::Cycle | Pattern::Type::KeepSingle;
    else if (timeSeparation <= 150 && positionSeparation < 20) convertTypeFlag |= Pattern::Type::ForceStack | Pattern::Type::LowProbability;
    else if (positionSeparation < 20 && density >= timingPoint.beatLength / 2.5) convertTypeFlag |= Pattern::Type::Reverse | Pattern::Type::LowProbability;
    else if (density < timingPoint.beatLength / 2.5 || effectPoint.KiaiMode)
    {
        // High density
    }
    else convertTypeFlag |= Pattern::Type::LowProbability;
}

Mania::Pattern Mania::PatternGenerator::generate()
{
    auto generateCore = [this]()->Pattern {
        Pattern pattern{ totalColumns };
        if (totalColumns == 1)
        {
            addToPattern(pattern, 0);
            return  { pattern };
        }

        int const lastColumn = previousPattern.hitObjects.empty() ? 0 : previousPattern.hitObjects.front()->getColumnIndex(totalColumns);

        if ((convertType & Pattern::Type::Reverse) && !previousPattern.hitObjects.empty())
        {
            for (int i = randomStart; i < totalColumns; ++i)
            {
                if (previousPattern.colunmHasObject(i))
                    addToPattern(pattern, randomStart + totalColumns - i - 1);
            }
            return { pattern };
        }

        if (convertType & Pattern::Type::Cycle
            && previousPattern.hitObjects.size() == 1
            && (totalColumns != 8 || lastColumn != 0)
            && (totalColumns % 2 == 0 || lastColumn != totalColumns / 2)
            )
        {
            int const column = randomStart + totalColumns - 1;
            addToPattern(pattern, column);
            return { pattern };
        }

        if (convertType & Pattern::Type::ForceStack && !previousPattern.hitObjects.empty())
        {
            for (int i = randomStart; i < totalColumns; ++i)
            {
                if (previousPattern.colunmHasObject(i))
                    addToPattern(pattern, i);
            }
            return { pattern };
        }

        if (previousPattern.hitObjects.size() == 1)
        {
            if (convertType & Pattern::Type::Stair)
            {
                // Generate a new pattern by placing on the next column, cycling back to the start if there is no "next"
                int targetColumn = lastColumn + 1;
                if (targetColumn == totalColumns)
                    targetColumn = randomStart;

                addToPattern(pattern, targetColumn);
                return { pattern };
            }

            if (convertType & Pattern::Type::ReverseStair)
            {
                // Generate a new pattern by placing on the previous column, cycling back to the end if there is no "previous"
                int targetColumn = lastColumn - 1;
                if (targetColumn == randomStart - 1)
                    targetColumn = totalColumns - 1;

                addToPattern(pattern, targetColumn);
                return { pattern };
            }

        }

        if (convertType & Pattern::Type::KeepSingle)
            return generateRandomNotes(1);

        auto const conversionDifficulty = getConversionDifficulty();
        if (convertType & Pattern::Type::Mirror)
        {
            if (conversionDifficulty > 6.5)
                return generateRandomPatternWithMirrored(0.12, 0.38, 0.12);
            if (conversionDifficulty > 4)
                return generateRandomPatternWithMirrored(0.12, 0.17, 0);

            return generateRandomPatternWithMirrored(0.12, 0, 0);
        }

        if (conversionDifficulty > 6.5)
        {
            if (convertType & Pattern::Type::LowProbability)
                return generateRandomPattern(0.78, 0.42, 0, 0);

            return generateRandomPattern(1, 0.62, 0, 0);
        }

        if (conversionDifficulty > 4)
        {
            if (convertType & Pattern::Type::LowProbability)
                return generateRandomPattern(0.35, 0.08, 0, 0);

            return generateRandomPattern(0.52, 0.15, 0, 0);
        }

        if (conversionDifficulty > 2)
        {
            if (convertType & Pattern::Type::LowProbability)
                return generateRandomPattern(0.18, 0, 0, 0);

            return generateRandomPattern(0.45, 0, 0, 0);
        }

        return generateRandomPattern(0, 0, 0, 0);
    };

    auto p = generateCore();

    for (auto const& obj : p.hitObjects)
    {
        if ((convertType & Pattern::Type::Stair) && obj->getColumnIndex(totalColumns) == totalColumns - 1)
            stairType = Pattern::Type::ReverseStair;
        if ((convertType & Pattern::Type::ReverseStair) && obj->getColumnIndex(totalColumns) == randomStart)
            stairType = Pattern::Type::Stair;

    }
    return p;
}


void Mania::PatternGenerator::addToPattern(Pattern& pattern, int column)
{
    pattern += std::make_unique<Circle>(HitObject::columnToX(column, totalColumns), 0, hitObject->time, hitObject->hitSample);
}

bool Mania::PatternGenerator::hasSpecialColumn() const
{
    return hitObject->hitSample.has(HitObject::HitSound::Clap) &&
        hitObject->hitSample.has(HitObject::HitSound::Finish);
}

int Mania::PatternGenerator::getNextColumn(int last) const
{
    if (convertType & Pattern::Type::Gathered)
    {
        ++last;
        if (last == totalColumns)
            last = randomStart;
    }
    else
        last = getRandomColumn();

    return last;
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

Mania::Pattern Mania::PatternGenerator::generateRandomNotes(int noteCount)
{
    Pattern pattern{ totalColumns };
    auto const allowStacking = !(convertType & Pattern::Type::ForceNotStack);

    if (!allowStacking)
        noteCount = std::min(noteCount, totalColumns - randomStart - previousPattern.numColumnWithObject());

    auto nextColumn = hitObject->getColumnIndex(totalColumns);
    for (int i = 0; i < noteCount; ++i)
    {
        nextColumn = 
    }

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

int Mania::PatternGenerator::getRandomNoteCountMirrored(double centreProbability, double p2, double p3, bool& addToCentre)
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


Mania::Pattern Mania::PatternGenerator::generateRandomPatternWithMirrored(double centreProbability, double p2, double p3)
{
    if(convertType & Pattern::Type::ForceNotStack)
        return generateRandomPattern(1 / 2.f + p2 / 2, p2, (p2 + p3) / 2, p3);

    Pattern pattern{ totalColumns };

    bool addToCentre{};
    auto const noteCount = getRandomNoteCountMirrored(centreProbability, p2, p3, addToCentre);
    auto const columnLimit = (totalColumns % 2 == 0 ? totalColumns : totalColumns - 1) / 2;
    auto nextColumn = getRandomColumn({}, columnLimit);

    for (int i = 0; i < noteCount; ++i)
    {
        nextColumn = findAvailableColumn(nextColumn, {}, columnLimit, )
    }
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
    std::function<int(int)> nextColumn,
    std::function<bool(int)> validator,
    std::vector<Pattern*> patterns
)
{
    auto isValid = [&validator, &patterns](int column)
    {
        auto const noOccupy = std::none_of(patterns.cbegin(), patterns.cend(), [column](Pattern const* p) { return p->colunmHasObject(column); });
        return validator ? validator && noOccupy : noOccupy;
    };

    auto const lowBound = lowerBound.value_or(randomStart);
    auto const upBound = upperBound.value_or(previousPattern.totalColumn);

    if (isValid(initialColumn))
        return initialColumn;

    bool hasValidColumns = false;
    for (int i = 0; i < lowBound; ++i)
    {
        hasValidColumns = isValid(i);
        if(hasValidColumns)
            break;
    }

    if (!hasValidColumns)
        return -1;

    // Iterate until a valid column is found. This is a random iteration in the default case.
    do
    {
        initialColumn = nextColumn ? nextColumn(initialColumn) : getRandomColumn(lowBound, upBound);
    } while (!isValid(initialColumn));

    return initialColumn;
}

double Mania::PatternGenerator::getConversionDifficulty() const
{
    auto drainTimeInSec = beatmap.getDrainTime() / 1'000;
    if (drainTimeInSec == 0)
        drainTimeInSec = 10'000;

    Difficulty const& baseDifficulty = beatmap.difficulty;
    double value = ((baseDifficulty.HPDrainRate + std::clamp(baseDifficulty.approachRate, 4.f, 7.f)) / 1.5 + beatmap.getCount() / drainTimeInSec * 9.f) / 38.f * 5.f / 1.15;
    value = std::min(value, 12.0);
    return value;
}

int Mania::PatternGenerator::getColumnCount() const
{
    auto const percentSliderOrSpinner = beatmap.getPercentOf<HitObject::Type::Slider, HitObject::Type::Spinner>();
    auto const roundedDifficulty = std::round(beatmap.difficulty.overallDifficulty);

    if (percentSliderOrSpinner < 0.2)
        return 7;
    else if (percentSliderOrSpinner < 0.3 || std::round(beatmap.difficulty.circleSize) >= 5)
        return roundedDifficulty > 5 ? 7 : 6;
    else if (percentSliderOrSpinner > 0.6)
        return roundedDifficulty > 4 ? 5 : 4;
    else
        return std::max(4, std::min(static_cast<int>(roundedDifficulty) + 1, 7));
}

OsuFile BeatmapConverter::convertBeatmap()
{
    return OsuFile{ 
        originalBeatmap.general,
        originalBeatmap.editor,
        originalBeatmap.metaData,
        originalBeatmap.events,
        originalBeatmap.timingPoints,
        originalBeatmap.colors,
        convertHitObjects() 
    };
}

std::vector<std::unique_ptr<HitObject>> BeatmapConverter::convertHitObjects()
{
    std::vector<std::unique_ptr<HitObject>> result;
    
    for (auto const& obj : const_cast<OsuFile&>(originalBeatmap))
    {
        result.emplace_back(convertHitObject(obj));
    }

    return result;
}

std::unique_ptr<HitObject> Mania::ManiaBeatmapConverter::convertHitObject(HitObject const& original)
{
    
}

void Mania::ManiaBeatmapConverter::recordNote(HitObject const& note)
{
    recordNote(note.time, Coord{ note.x, note.y });
}

void Mania::ManiaBeatmapConverter::recordNote(int time, Coord position)
{
    lastTime = time;
    lastPosition = position;
}

void Mania::ManiaBeatmapConverter::computeDensity(int newNoteTime)
{
    if (prevNoteTimes.size() == MaxNotesForDensity)
        prevNoteTimes.erase(prevNoteTimes.begin());
    
    prevNoteTimes.push_back(newNoteTime);

    if (prevNoteTimes.size() >= 2)
        density = (prevNoteTimes.back() - prevNoteTimes.front()) / static_cast<double>(prevNoteTimes.size());
}

std::vector<std::unique_ptr<HitObject>> Mania::ManiaBeatmapConverter::generateConverted(HitObject const& original)
{
    /*
    * In osu lazer, it's 
    * switch(original)
    * {
    *     case IHasDistance:
    *     case IHasDuration
    *     case IHasPosition:
    * }
    */

    switch (original.type)
    {
        case HitObject::Type::Circle:
        case HitObject::Type::Slider:
            computeDensity(original.time);

        case HitObject::Type::Spinner:
        default:
            break;
    }
    return std::vector<std::unique_ptr<HitObject>>();
}
