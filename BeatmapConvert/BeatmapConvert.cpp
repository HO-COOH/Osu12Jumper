#include "include/BeatmapConvert.hpp"
#include <algorithm>
#include <cassert>
#include "RandomEngine.hpp"


Mania::HitObjectPatternGenerator::HitObjectPatternGenerator(
    HitObject const& hitObject, 
    OsuFile& beatmap, 
    Pattern&& previousPattern, 
    int previousTime, 
    Coord previousPosition, 
    double density, 
    Pattern::Type lastStair, 
    OsuFile const& originalBeatmap,
    int totalColumns
)
    : 
    PatternGenerator(std::move(previousPattern), hitObject, originalBeatmap, totalColumns),
    beatmap{beatmap},
    stairType{lastStair}
{
    auto const& timingPoint = originalBeatmap.getTimingPointAt<TimingPoint::Type::TimingControlPoint>(hitObject.time);

    
    Coord const positionData{ hitObject.x, hitObject.y };
    
    auto const positionSeparation = positionData.distanceTo(previousPosition);
    auto const timeSeparation = hitObject.time - previousTime;

    auto& convertTypeFlag = reinterpret_cast<unsigned&>(convertType);
    
    if (timeSeparation <= 80)       convertTypeFlag |= Pattern::Type::ForceNotStack | Pattern::Type::KeepSingle;
    else if (timeSeparation <= 95)  convertTypeFlag |= Pattern::Type::ForceNotStack | Pattern::Type::KeepSingle | lastStair;
    else if (timeSeparation <= 105) convertTypeFlag |= Pattern::Type::ForceNotStack | Pattern::Type::LowProbability;
    else if (timeSeparation <= 125) convertTypeFlag |= Pattern::Type::ForceNotStack;
    else if (timeSeparation <= 135 && positionSeparation < 20) convertTypeFlag |= Pattern::Type::Cycle | Pattern::Type::KeepSingle;
    else if (timeSeparation <= 150 && positionSeparation < 20) convertTypeFlag |= Pattern::Type::ForceStack | Pattern::Type::LowProbability;
    else if (positionSeparation < 20 && density >= timingPoint.beatLength / 2.5) convertTypeFlag |= Pattern::Type::Reverse | Pattern::Type::LowProbability;
    else if (density < timingPoint.beatLength / 2.5 || timingPoint.effects & static_cast<unsigned>(TimingPoint::Effect::Kiai))
    {
        // High density
    }
    else convertTypeFlag |= Pattern::Type::LowProbability;

    if (!(convertType & Pattern::Type::KeepSingle))
    {
        if (hitObject.hitSound == HitObject::HitSound::Finish && totalColumns != 8)
            convertTypeFlag |= Pattern::Type::Mirror;
        else if (hitObject.hitSound == HitObject::HitSound::Clap)
            convertTypeFlag |= Pattern::Type::Gathered;
    }
}

//#define PrintDetail

Mania::Pattern Mania::HitObjectPatternGenerator::generate()
{
#ifdef PrintDetail
    std::cout << "original: " << hitObject.time << ", type: " << hitObject.type << " -> convertType: " << convertType << '\n';
#endif

    auto generateCore = [this]()->Pattern 
    {
        Pattern pattern{ totalColumns };
        if (totalColumns == 1)
        {
            addToPattern(pattern, 0);
            return pattern;
        }

        int const lastColumn = previousPattern.hitObjects.empty() ? 0 : previousPattern.hitObjects.front()->getColumnIndex(totalColumns);

        if ((convertType & Pattern::Type::Reverse) && !previousPattern.hitObjects.empty())
        {
            for (int i = randomStart; i < totalColumns; ++i)
            {
                if (previousPattern.colunmHasObject(i))
                    addToPattern(pattern, randomStart + totalColumns - i - 1);
            }
            return pattern;
        }

        if (convertType & Pattern::Type::Cycle
            && previousPattern.hitObjects.size() == 1
            && (totalColumns != 8 || lastColumn != 0)
            && (totalColumns % 2 == 0 || lastColumn != totalColumns / 2)
            )
        {
            int const column = randomStart + totalColumns - 1;
            addToPattern(pattern, column);
            return pattern;
        }

        if (convertType & Pattern::Type::ForceStack && !previousPattern.hitObjects.empty())
        {
            for (int i = randomStart; i < totalColumns; ++i)
            {
                if (previousPattern.colunmHasObject(i))
                    addToPattern(pattern, i);
            }
            return pattern;
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
                return pattern;
            }

            if (convertType & Pattern::Type::ReverseStair)
            {
                // Generate a new pattern by placing on the previous column, cycling back to the end if there is no "previous"
                int targetColumn = lastColumn - 1;
                if (targetColumn == randomStart - 1)
                    targetColumn = totalColumns - 1;

                addToPattern(pattern, targetColumn);
                return pattern;
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
#ifdef PrintDetail
    for (auto const& obj : p.hitObjects)
        std::cout <<obj->getColumnIndex(totalColumns) << " : " << *obj << '\n';
#endif
    return p;
}


void Mania::HitObjectPatternGenerator::addToPattern(Pattern& pattern, int column) const
{
    //pattern += std::make_unique<Circle>(HitObject::ColumnToX(column, totalColumns), 0, hitObject->time, hitObject->hitSample);
    pattern += Circle::MakeManiaHitObject(column, totalColumns, hitObject.time, hitObject.hitSound, hitObject.hitSample);
}

bool Mania::HitObjectPatternGenerator::hasSpecialColumn() const
{
    return 
        hitObject.hitSound == HitObject::HitSound::Clap &&
        hitObject.hitSound == HitObject::HitSound::Finish;
}

int Mania::HitObjectPatternGenerator::getNextColumn(int last) const
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



Mania::Pattern Mania::HitObjectPatternGenerator::generateRandomNotes(int noteCount) const
{
    Pattern pattern{ totalColumns };
    auto const allowStacking = !(convertType & Pattern::Type::ForceNotStack);

    if (!allowStacking)
        noteCount = std::min(noteCount, totalColumns - randomStart - previousPattern.numColumnWithObject());

    auto nextColumn = hitObject.getColumnIndex(totalColumns);

    auto getNextColumn = [this](int last)
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
    };

    for (int i = 0; i < noteCount; ++i)
    {
        nextColumn = allowStacking
            ? findAvailableColumn(nextColumn, {}, {}, getNextColumn, {}, { &pattern })
            : findAvailableColumn(nextColumn, {}, {}, getNextColumn, {}, { &pattern, &previousPattern });

        addToPattern(pattern, nextColumn);
    }

    return pattern;

}

int Mania::HitObjectPatternGenerator::getRandomNoteCount(double p2, double p3, double p4, double p5) const
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

    if (hitObject.hitSound == HitObject::HitSound::Clap)
        p2 = 1;

    return GetRandomNoteCount(p2, p3, p4, p5);
}

int Mania::HitObjectPatternGenerator::getRandomNoteCountMirrored(double centreProbability, double p2, double p3, bool& addToCentre)
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


Mania::Pattern Mania::HitObjectPatternGenerator::generateRandomPatternWithMirrored(double centreProbability, double p2, double p3)
{
    if(convertType & Pattern::Type::ForceNotStack)
        return generateRandomPattern(1 / 2.f + p2 / 2, p2, (p2 + p3) / 2, p3);

    Pattern pattern{ totalColumns };

    bool addToCentre{};
    auto const noteCount = getRandomNoteCountMirrored(centreProbability, p2, p3, addToCentre);
    auto const columnLimit = (totalColumns % 2 == 0 ? totalColumns : totalColumns - 1) / 2 - 1; 
                                                            //osu lazer differs here        ^
                                                            //Our limit is inclusive
    auto nextColumn = getRandomColumn({}, columnLimit);

    for (int i = 0; i < noteCount; ++i)
    {
        nextColumn = findAvailableColumn(nextColumn, {}, columnLimit, {}, {}, { &pattern });

        // Add normal note
        addToPattern(pattern, nextColumn);

        // Add mirrored note
        auto const mirrorColumn = randomStart + totalColumns - nextColumn - 1;
        addToPattern(pattern, mirrorColumn);

        assert(nextColumn != mirrorColumn);
    }

    if (addToCentre)
        addToPattern(pattern, totalColumns / 2);

    if (randomStart > 0 && hasSpecialColumn())
        addToPattern(pattern, 0);

    return pattern;
}

Mania::Pattern Mania::HitObjectPatternGenerator::generateRandomPattern(double p2, double p3, double p4, double p5) const
{
    Pattern pattern{ totalColumns };
    
    pattern += generateRandomNotes(getRandomNoteCount(p2, p3, p4, p5));

    if (randomStart > 0 && hasSpecialColumn())
        addToPattern(pattern, 0);

    return pattern;
}

Mania::ManiaBeatmapConverter::ManiaBeatmapConverter(OsuFile const& originalBeatmap) 
    : BeatmapConverter{ originalBeatmap }, 
    beatmap{ originalBeatmap },
    targetColumns{ getTargetColumn() }
{

}

OsuFile Mania::ManiaBeatmapConverter::convertBeatmap()
{
    auto f = BeatmapConverter::convertBeatmap();
    f.difficulty.circleSize = static_cast<float>(targetColumns);
    f.general.mode = Mode::Mania;
    return f;
}

int Mania::ManiaBeatmapConverter::getTargetColumn() const
{
    auto const percentSliderOrSpinner = originalBeatmap.getPercentOf<HitObject::Type::Slider, HitObject::Type::Spinner>();
    auto const roundedDifficulty = std::round(originalBeatmap.difficulty.overallDifficulty);

    if (percentSliderOrSpinner < 0.2)
        return 7;
    else if (percentSliderOrSpinner < 0.3 || std::round(originalBeatmap.difficulty.circleSize) >= 5)
        return roundedDifficulty > 5 ? 7 : 6;
    else if (percentSliderOrSpinner > 0.6)
        return roundedDifficulty > 4 ? 5 : 4;
    else
        return std::max(4, std::min(static_cast<int>(roundedDifficulty) + 1, 7));
}

//void Mania::ManiaBeatmapConverter::cleanUpStackedNotes(std::vector<std::unique_ptr<HitObject>>& result) const
//{
//    std::vector<bool> currentColumnOccupied(targetColumns);
//    for()
//}

Mania::ManiaBeatmapConverter& Mania::ManiaBeatmapConverter::setTargetColumn(int target)
{
    if (target <= 0 || target > 9)
        throw std::invalid_argument{ "Invalid target columns" };
    targetColumns = target;
    return *this;
}

void Mania::ManiaBeatmapConverter::handleNewPattern(Pattern pattern, Pattern::Type stairType, std::vector<std::unique_ptr<HitObject>>& result)
{
    /*update lastStair*/
    lastStair = stairType;
    
    /*insert into result vector*/
    result.reserve(result.size() + pattern.hitObjects.size());
    std::transform(
        pattern.hitObjects.cbegin(), 
        pattern.hitObjects.cend(), 
        std::back_inserter(result),
        [](std::unique_ptr<HitObject> const& obj)
        {
            return obj->clone();
        }
    );

    /*update lastPattern*/
    lastPattern.emplace(std::move(pattern));
}

OsuFile BeatmapConverter::convertBeatmap()
{
    return OsuFile{ 
        originalBeatmap.general,
        originalBeatmap.editor,
        originalBeatmap.metaData,
        originalBeatmap.difficulty,
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
        //result.emplace_back(convertHitObject(obj));
        auto converted = convertHitObject(obj);
        std::move(converted.begin(), converted.end(), std::back_inserter(result));
    }

    return result;
}

std::vector<std::unique_ptr<HitObject>> Mania::ManiaBeatmapConverter::convertHitObject(HitObject const& original)
{
    return generateConverted(original);
}

//OsuFile Mania::ManiaBeatmapConverter::convertBeatmap()
//{
//    return originalBeatmap;
//}


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
    *     case IHasDuration: -> Slider
    *     case IHasPosition: -> Circle
    * }
    */

    std::vector<std::unique_ptr<HitObject>> result;
    switch (original.type)
    {
        case HitObject::Type::Circle:   //IHasPosition
        {
            computeDensity(original.time);
            HitObjectPatternGenerator conversion{ 
                original, 
                beatmap, 
                lastPattern.has_value()? std::move(*lastPattern) : Pattern{targetColumns}, 
                lastTime, 
                lastPosition, 
                density, 
                lastStair, 
                originalBeatmap,
                targetColumns
            };
            recordNote(original.time, Coord{ original.x, original.y });
            handleNewPattern(conversion.generate(), conversion.stairType, result);
            break;
        }
        case HitObject::Type::Slider:   //IHasDistance
        {
            DistanceObjectPatternGenerator conversion{
                original,
                beatmap,
                lastPattern.has_value()? std::move(*lastPattern) : Pattern{targetColumns},
                originalBeatmap,
                targetColumns
            };

            for (int i = 0; i <= conversion.spanCount; ++i)
            {
                auto const time = original.time + conversion.segmentDuration * i;
                recordNote(time, { original.x, original.y });
                computeDensity(time);
            }
            handleNewPattern(conversion.generate(), conversion.convertType, result);
            break;
        }
        case HitObject::Type::Spinner:  //IHasEndTime
        {
        }
        default:
            break;
    }

    return result;
}

Mania::Pattern Mania::DistanceObjectPatternGenerator::generate()
{
    assert(startTime == hitObject.time);
#ifdef PrintDetail
    //std::cout << "original: " << hitObject.time <<", endTime: "<< endTime << ", type: " << hitObject.type << " -> convertType: " << convertType << '\n';
#endif

    if (totalColumns == 1)
    {
        Pattern pattern{ totalColumns };
        //pattern += std::make_unique<Hold>(HitObject::ColumnToX(0, totalColumns), 192, startTime, HitObject::HitSound{}, endTime, HitObject::HitSample{});
        addToPattern(pattern, 0, startTime, endTime);
        return pattern;
    }

    auto& convertTypeFlag = reinterpret_cast<unsigned&>(convertType);

    auto const conversionDifficulty = getConversionDifficulty();

    if (spanCount > 1)
    {
        if (segmentDuration <= 90)
            return generateRandomHoldNotes(startTime, 1);

        if (segmentDuration <= 120)
        {
            convertTypeFlag |= Pattern::Type::ForceNotStack;
            return generateRandomNotes(startTime, spanCount + 1);
        }

        if (segmentDuration <= 160)
            return generateStair(startTime);

        if (segmentDuration <= 200 && conversionDifficulty > 3)
            return generateRandomMultipleNotes(startTime);

        auto duration = endTime - startTime;
        if (duration >= 4000)
            return generateNRandomNotes(startTime, 0.23, 0, 0);

        if (segmentDuration > 400 && spanCount < totalColumns - 1 - randomStart)
            return generateTiledHoldNotes(startTime);

        return generateHoldAndNormalNotes(startTime);
    }

    if (segmentDuration <= 110)
    {
        if (previousPattern.numColumnWithObject() < totalColumns)
            convertTypeFlag |= Pattern::Type::ForceNotStack;
        else
            convertTypeFlag &= ~Pattern::Type::ForceNotStack;
        return generateRandomNotes(startTime, segmentDuration < 80 ? 1 : 2);
    }

    if (conversionDifficulty > 6.5)
    {
        if (convertType & (Pattern::Type::LowProbability))
            return generateNRandomNotes(startTime, 0.78, 0.3, 0);

        return generateNRandomNotes(startTime, 0.85, 0.36, 0.03);
    }

    if (conversionDifficulty > 4)
    {
        if (convertType & (Pattern::Type::LowProbability))
            return generateNRandomNotes(startTime, 0.43, 0.08, 0);

        return generateNRandomNotes(startTime, 0.56, 0.18, 0);
    }

    if (conversionDifficulty > 2.5)
    {
        if (convertType & (Pattern::Type::LowProbability))
            return generateNRandomNotes(startTime, 0.3, 0, 0);

        return generateNRandomNotes(startTime, 0.37, 0.08, 0);
    }

    if (convertType & (Pattern::Type::LowProbability))
        return generateNRandomNotes(startTime, 0.17, 0, 0);

    return generateNRandomNotes(startTime, 0.27, 0, 0);
}

Mania::DistanceObjectPatternGenerator::DistanceObjectPatternGenerator(HitObject const& hitObject, OsuFile& beatmap, Pattern&& previousPattern, OsuFile const& originalBeatmap, int totalColumns)
    : PatternGenerator(std::move(previousPattern), hitObject, originalBeatmap, totalColumns),
    startTime{hitObject.time},
    spanCount{ dynamic_cast<Slider const&>(hitObject).slides},
    endTime{ static_cast<int>(getEndTime())},
    segmentDuration{ static_cast<int>(getSegmentDuration())},
    convertType{ Pattern::Type::LowProbability }
{
    //assert(segmentDuration >= originalBeatmap.timingPoints.front().beatLength / 32);
}

Mania::Pattern Mania::DistanceObjectPatternGenerator::generateRandomHoldNotes(int startTime, int noteCount)
{
    Pattern pattern{ totalColumns };

    int usableColumns = totalColumns - randomStart - previousPattern.numColumnWithObject();
    int nextColumn = getRandomColumn();

    for (int i = 0; i < std::min(usableColumns, noteCount); ++i)
    {
        // Find available column
        nextColumn = findAvailableColumn(nextColumn, { &pattern, &previousPattern });
        //pattern += std::make_unique<Hold>(HitObject::ColumnToX(nextColumn, totalColumns), 192, startTime, HitObject::HitSound{}, endTime, HitObject::HitSample{});
        addToPattern(pattern, nextColumn, startTime, endTime);
    }

    // This is can't be combined with the above loop due to RNG
    for (int i = 0; i < noteCount - usableColumns; i++)
    {
        nextColumn = findAvailableColumn(nextColumn, { &pattern });
        //pattern += std::make_unique<Hold>(HitObject::ColumnToX(nextColumn, totalColumns), 192, startTime, HitObject::HitSound{}, endTime, HitObject::HitSample{});
        addToPattern(pattern, nextColumn, startTime, endTime);
    }

    return pattern;
}

Mania::Pattern Mania::DistanceObjectPatternGenerator::generateRandomNotes(int startTime, int noteCount) 
{
    Pattern pattern{ totalColumns };

    int nextColumn = hitObject.getColumnIndex(totalColumns);

    if ((convertType & Pattern::Type::ForceNotStack) && previousPattern.numColumnWithObject() < totalColumns)
        nextColumn = findAvailableColumn(nextColumn, { &previousPattern });

    int lastColumn = nextColumn;

    for (int i = 0; i < noteCount; i++)
    {
        //pattern += std::make_unique<Hold>(HitObject::ColumnToX(nextColumn, totalColumns), 192, startTime, HitObject::HitSound{}, endTime, HitObject::HitSample{});
        addToPattern(pattern, nextColumn, startTime, endTime);
        nextColumn = findAvailableColumn(nextColumn, {}, {}, {}, [lastColumn](int c) { return c != lastColumn; }, {});
        lastColumn = nextColumn;
        startTime += segmentDuration;
    }

    return pattern;
}

Mania::Pattern Mania::DistanceObjectPatternGenerator::generateStair(int startTime) 
{
    // - - - -
    // x - - -
    // - x - -
    // - - x -
    // - - - x
    // - - x -
    // - x - -
    // x - - -

    Pattern pattern{ totalColumns };

    int column = hitObject.getColumnIndex(totalColumns);
    bool increasing = RandomEngine::getRand() > 0.5;

    for (int i = 0; i <= spanCount; i++)
    {
        //pattern += std::make_unique<Circle>(HitObject::ColumnToX(column, totalColumns), 192, startTime, HitObject::HitSound{}, HitObject::HitSample{});
        addToPattern(pattern, column, startTime);
        startTime += segmentDuration;

        // Check if we're at the borders of the stage, and invert the pattern if so
        if (increasing)
        {
            if (column >= totalColumns - 1)
            {
                increasing = false;
                column--;
            }
            else
                column++;
        }
        else
        {
            if (column <= randomStart)
            {
                increasing = true;
                column++;
            }
            else
                column--;
        }
    }

    return pattern;
}

Mania::Pattern Mania::DistanceObjectPatternGenerator::generateRandomMultipleNotes(int startTime) 
{
    // - - - -
    // x - - -
    // - x x -
    // - - - x
    // x - x -

    Pattern pattern{ totalColumns };

    bool legacy = totalColumns >= 4 && totalColumns <= 8;
    int interval = RandomEngine::getRand(Range{ 1, totalColumns - (legacy ? 1 : 0) });

    int nextColumn = hitObject.getColumnIndex(totalColumns);

    for (int i = 0; i <= spanCount; i++)
    {
        //pattern += std::make_unique<Circle>(HitObject::ColumnToX(nextColumn, totalColumns), 192, startTime, HitObject::HitSound{}, HitObject::HitSample{});
        addToPattern(pattern, nextColumn, startTime);

        nextColumn += interval;
        if (nextColumn >= totalColumns - randomStart)
            nextColumn = nextColumn - totalColumns - randomStart + (legacy ? 1 : 0);
        nextColumn += randomStart;

        // If we're in 2K, let's not add many consecutive doubles
        if (totalColumns > 2)
            //pattern += std::make_unique<Circle>(HitObject::ColumnToX(nextColumn, totalColumns), 192, startTime, HitObject::HitSound{}, HitObject::HitSample{});
            addToPattern(pattern, nextColumn, startTime);

        nextColumn = getRandomColumn();
        startTime += segmentDuration;
    }

    return pattern;
}

Mania::Pattern Mania::DistanceObjectPatternGenerator::generateNRandomNotes(int startTime, double p2, double p3, double p4) 
{
    // - - - -
    // �� - �� ��
    // �� - �� ��
    // �� - �� ��

    switch (totalColumns)
    {
        case 2:
            p2 = 0;
            p3 = 0;
            p4 = 0;
            break;

        case 3:
            p2 = std::min(p2, 0.1);
            p3 = 0;
            p4 = 0;
            break;

        case 4:
            p2 = std::min(p2, 0.3);
            p3 = std::min(p3, 0.04);
            p4 = 0;
            break;

        case 5:
            p2 = std::min(p2, 0.34);
            p3 = std::min(p3, 0.1);
            p4 = std::min(p4, 0.03);
            break;
    }

    constexpr auto isDoubleSample = [](HitObject::HitSound sample) { return sample == HitObject::HitSound::Clap || sample == HitObject::HitSound::Finish; };

    bool canGenerateTwoNotes = !(convertType & (Pattern::Type::LowProbability));
    canGenerateTwoNotes &= isDoubleSample(hitObject.hitSound) /*|| sampleInfoListAt(StartTime).Any(isDoubleSample)*/;

    if (canGenerateTwoNotes)
        p2 = 1;

    return generateRandomHoldNotes(startTime, GetRandomNoteCount(p2, p3, p4));
}

Mania::Pattern Mania::DistanceObjectPatternGenerator::generateTiledHoldNotes(int startTime) 
{
    // - - - -
    // �� �� �� ��
    // �� �� �� ��
    // �� �� �� ��
    // �� �� �� ��
    // �� �� �� -
    // �� �� - -
    // �� - - -

    Pattern pattern{totalColumns};

    int columnRepeat = std::min(spanCount, totalColumns);

    // Due to integer rounding, this is not guaranteed to be the same as EndTime (the class-level variable).
    int endTime = startTime + segmentDuration * spanCount;

    int nextColumn = hitObject.getColumnIndex(totalColumns);
    if (convertType & (Pattern::Type::ForceNotStack) && previousPattern.numColumnWithObject() < totalColumns)
        nextColumn = findAvailableColumn(nextColumn, { &previousPattern });

    for (int i = 0; i < columnRepeat; i++)
    {
        nextColumn = findAvailableColumn(nextColumn, { &pattern });
        //pattern += std::make_unique<Hold>(HitObject::ColumnToX(nextColumn, totalColumns), 192, startTime, HitObject::HitSound{}, endTime, HitObject::HitSample{});
        addToPattern(pattern, nextColumn, startTime, endTime);
        startTime += segmentDuration;
    }

    return pattern;
}

Mania::Pattern Mania::DistanceObjectPatternGenerator::generateHoldAndNormalNotes(int startTime) 
{
    // - - - -
    // �� x x -
    // �� - x x
    // �� x - x
    // �� - x x

    Pattern pattern{ totalColumns };

    int holdColumn = hitObject.getColumnIndex(totalColumns);
    if (convertType & (Pattern::Type::ForceNotStack) && previousPattern.numColumnWithObject() < totalColumns)
        holdColumn = findAvailableColumn(holdColumn, { &previousPattern });

    // Create the hold note
    //pattern += std::make_unique<Hold>(HitObject::ColumnToX(holdColumn, totalColumns), 192, startTime, HitObject::HitSound{}, endTime, HitObject::HitSample{});
    addToPattern(pattern, holdColumn, startTime, endTime);

    int nextColumn = getRandomColumn();
    int noteCount;

    auto const conversionDifficulty = getConversionDifficulty();

    if (getConversionDifficulty() > 6.5)
        noteCount = GetRandomNoteCount(0.63, 0);
    else if (conversionDifficulty > 4)
        noteCount = GetRandomNoteCount(totalColumns < 6 ? 0.12 : 0.45, 0);
    else if (conversionDifficulty > 2.5)
        noteCount = GetRandomNoteCount(totalColumns < 6 ? 0 : 0.24, 0);
    else
        noteCount = 0;
    noteCount = std::min(totalColumns - 1, noteCount);

    //bool ignoreHead = !sampleInfoListAt(startTime).Any(s = > s.Name == HitSampleInfo.HIT_WHISTLE || s.Name == HitSampleInfo.HIT_FINISH || s.Name == HitSampleInfo.HIT_CLAP);
    bool ignoreHead = !(
        hitObject.hitSound == HitObject::HitSound::Whistle ||
        hitObject.hitSound == HitObject::HitSound::Finish ||
        hitObject.hitSound == HitObject::HitSound::Clap
    );

    Pattern rowPattern{ totalColumns };

    for (int i = 0; i <= spanCount; i++)
    {
        if (!(ignoreHead && startTime == this->startTime))
        {
            for (int j = 0; j < noteCount; j++)
            {
                nextColumn = findAvailableColumn(nextColumn, {}, {}, {}, [holdColumn](int c) { return c != holdColumn; }, { &rowPattern });
                //rowPattern += std::make_unique<Circle>(HitObject::ColumnToX(nextColumn, totalColumns), 192, startTime, HitObject::HitSound{}, HitObject::HitSample{});
                addToPattern(rowPattern, nextColumn, startTime);
            }
        }

        pattern += std::move(rowPattern);

        startTime += segmentDuration;
    }

    return pattern;
}

double Mania::DistanceObjectPatternGenerator::getEndTime() const
{
    auto const& slider = dynamic_cast<Slider const&>(hitObject);

    auto const beatLength = originalBeatmap.timingPoints.front().beatLength;
    auto const bpmMultiplier = beatLength < 0 ? std::clamp(-beatLength, 10.f, 10000.f) / 100.0 : 1;
    return std::floor(startTime + static_cast<double>(slider.length) * beatLength * spanCount * 0.01 / originalBeatmap.difficulty.sliderMultiplier);
                                                        //160 / 
}

double Mania::DistanceObjectPatternGenerator::getSegmentDuration() const
{
    assert(endTime != 0);
    return (endTime - startTime) / spanCount;
}

void Mania::DistanceObjectPatternGenerator::addToPattern(
    Pattern& pattern,
    int columnIndex,
    int startTime,
    HitObject::HitSound hitSound,
    HitObject::HitSample hitSample)
{
    pattern += std::make_unique<Circle>(HitObject::ColumnToX(columnIndex, totalColumns), 192, startTime, hitSound, std::move(hitSample));
}

void Mania::DistanceObjectPatternGenerator::addToPattern(
    Pattern& pattern,
    int columnIndex,
    int startTime,
    int endTime,
    HitObject::HitSound hitSound,
    HitObject::HitSample hitSample)
{
    /*
        prevent hold note being too short
        This is usually caused by the conversion error between float/double <-> int
    */
    //assert(endTime > startTime);
    if (endTime - startTime <= originalBeatmap.timingPoints.front().beatLength / 32)
        addToPattern(pattern, columnIndex, startTime, hitSound, std::move(hitSample));
    else
        pattern += std::make_unique<Hold>(HitObject::ColumnToX(columnIndex, totalColumns), 192, startTime, hitSound, endTime, std::move(hitSample));
}

#include <future>

inline std::string& toLowerInplace(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

inline std::string toLowerInplace(std::string&& str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

static inline bool ShouldConvert(std::filesystem::directory_entry const& entry)
{
    return entry.path().extension() == ".osu" && toLowerInplace(entry.path().filename().string()).find("convert") == std::string::npos;
}

std::future<void> Mania::ConvertImpl(std::filesystem::directory_entry const& entry)
{
    return
        std::async(
            std::launch::async,
            [entry]()
            {
                std::cout << "Converting " << entry << '\n';
                OsuFile f{ std::ifstream{entry.path()} };
                Mania::ManiaBeatmapConverter converter{ f };
                converter.setTargetColumn(4);

                auto convertedMap = converter.convertBeatmap();
                convertedMap.metaData.version += "Converted";

                {
                    std::cout << "\nGenerate:\n\t"
                        << convertedMap.getCount<HitObject::Type::Circle>() << " circles\n"
                        << '\t' << convertedMap.getCount<HitObject::Type::Hold>() << " holds \n"
                        << '\t';
                    for (int i = 0; i < convertedMap.difficulty.circleSize; ++i)
                        std::cout << convertedMap.getPercentOfHitObjectInColumn(i) * 100.f << "%   ";
                }

                Mania::AddBreaks(convertedMap);
                Mania::RemoveShortHolds(convertedMap);
                convertedMap.metaData.version += "Break";

                auto fileName = convertedMap.getSaveFileName();
                auto rootDir = entry.path().parent_path().string();

                constexpr auto IsRootCurrent = [](std::string const& dir)
                {
                    return dir.empty() || dir == ".";
                };

                #ifdef WIN32
                    auto path = IsRootCurrent(rootDir)? fileName : rootDir + "\\" + fileName;
                #else
                    auto path = IsRootCurrent(rootDir)? fileName : rootDir + "/" + fileName;
                #endif
                try 
                {
                    convertedMap.save(path.c_str());
                }
                catch (std::exception const& e)
                {
                    std::cerr << e.what() <<" Retrying!"<< '\n';
                    
                    /*Maybe because of file too long, make shorter then retry*/
                    convertedMap.metaData.version = "test";
                    fileName = convertedMap.getSaveFileName();

                    #ifdef WIN32
                        path = IsRootCurrent(rootDir) ? fileName : rootDir + "\\" + fileName;
                    #else
                        path = IsRootCurrent(rootDir) ? fileName : rootDir + "/" + fileName;
                    #endif
                    try 
                    {
                        convertedMap.save(path.c_str());
                    }
                    catch(std::exception const& e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }

            }
        );
    
}

#include <type_traits>
template<typename DirectoryIterator>
void ConvertAllImpl(DirectoryIterator&& dir)
{
    std::vector<std::future<void>> futures;
    for (auto&& entry : dir)
    {
        /*Do not convert maps that's already converted*/
        if (ShouldConvert(entry))
            futures.emplace_back(Mania::ConvertImpl(entry));
    }

    for (auto& future : futures)
        future.get();
}

void Mania::ConvertAll(std::filesystem::recursive_directory_iterator dir)
{
    ConvertAllImpl(dir);
}

void Mania::ConvertAll(std::filesystem::directory_iterator dir)
{
    ConvertAllImpl(dir);
}

void Mania::ConvertAll(std::filesystem::path path)
{
    ConvertAll(std::filesystem::recursive_directory_iterator{ std::move(path) });
}

/**
 * @return true if the advancing can be done, false if it reached pass end
 */
template<typename Iter>
static bool AdvanceEndIter(int startTime, Iter& iter, Iter const& endIter, int windowSizeInMilliseconds)
{
    while(iter < endIter && (*iter)->time < startTime + windowSizeInMilliseconds)
        iter++;
    return iter < endIter;
}

/**
 * @return true if the window needs to be removed
 */
template<typename Iter>
static bool CheckWindow(OsuFile const& beatmap, Iter const& start, Iter& end, int windowSizeMilliseconds, int hitObjectCountsThreshold)
{
    auto const startTime = (*start)->time;
    auto endTime = (*end)->time;
    while(end < beatmap.hitObjects.cend() && (beatmap.getNumHitObjectDuring(startTime, endTime) < hitObjectCountsThreshold && endTime - startTime < windowSizeMilliseconds))
    {
        ++end;
        if (end >= beatmap.hitObjects.cend())
            return false;
        endTime = (*end)->time;
    }
    /* either there is enough hit objects or window has reached*/
    auto const numHitObject = beatmap.getNumHitObjectDuring(startTime, endTime);
    if (numHitObject <= hitObjectCountsThreshold && endTime - startTime >= windowSizeMilliseconds)
    {
        while (end < beatmap.hitObjects.cend() && (beatmap.getNumHitObjectDuring(startTime, endTime) <= hitObjectCountsThreshold))
        {
            ++end;
            if (end >= beatmap.hitObjects.cend())
                return false;
            endTime = (*end)->time;
        }
        return true;
    }
    else
        return false;
}

template<typename Iter>
static auto MakeBreak(OsuFile& beatmap, Iter start, Iter end)
{
    assert(start != end);
    auto const startTime = (*start)->time;
    auto const endTime = (*end)->time;
    /*Remove the hit objects*/
    std::cerr << "Removed [" << startTime << " , " << endTime << "] hitobjects = " << beatmap.getNumHitObjectDuring(startTime, endTime) <<'\n';
    beatmap.hitObjects.erase(start, end);
    

    /*Insert break section*/
    auto iter = beatmap.events.getEventAt<Events::Type::Break>(startTime);
    beatmap.events.breaks.insert(iter, Break{ startTime, endTime });

    return std::distance(start, end);
}

void Mania::AddBreaks(OsuFile& beatmap, int windowSizeInBeats, int hitObjectCountsThreshold)
{
    /*Don't convert if there are few hitobjects or map length too short*/
    if (beatmap.hitObjects.size() <= hitObjectCountsThreshold || beatmap.timingPoints.empty())
        return;

    auto const beatLength = beatmap.timingPoints.front().beatLength;
    if ((beatmap.getDrainTime() / beatLength) < windowSizeInBeats)
        return;

    /*Do convert*/
    auto const windowSizeInMilliseconds = windowSizeInBeats * beatLength;
    auto windowStartIter = beatmap.hitObjects.begin();
    auto windowEndIter = windowStartIter;
    
    AdvanceEndIter((*windowStartIter)->time, windowEndIter, beatmap.hitObjects.end(), windowSizeInMilliseconds);
    while(windowEndIter != beatmap.hitObjects.end())
    {
        auto index = std::distance(beatmap.hitObjects.begin(), windowStartIter);
        
        if (CheckWindow(beatmap, windowStartIter, windowEndIter, windowSizeInMilliseconds, hitObjectCountsThreshold))
        {
            MakeBreak(beatmap, windowStartIter, windowEndIter);
        }
        else
            ++index;
        

        /*reset the iterators*/
        windowStartIter = beatmap.hitObjects.begin() + index;
        windowEndIter = windowStartIter;
        if(++windowEndIter >= beatmap.hitObjects.end())
            break;
    }
}


static inline bool isOneFourthHold(Hold* hold, int beatLength)
{
    return abs(hold->getDuration() - beatLength / 4) <= beatLength / 16;
}

void Mania::RemoveShortHolds(OsuFile& beatmap)
{
    auto columns = beatmap.difficulty.circleSize;
    //convert all one-fourth holds in hitObjects to circles
    for (auto& hitObject : beatmap.hitObjects)
    {
        if (hitObject->type == HitObject::Type::Hold)
        {
            auto holdPtr = dynamic_cast<Hold*>(hitObject.get());
            if (isOneFourthHold(holdPtr, beatmap.timingPoints.front().beatLength))
            {
                hitObject.reset(new Circle{ Circle::ColumnToX(holdPtr->getColumnIndex(columns), columns), 192, holdPtr->time, HitObject::HitSound{}, HitObject::HitSample{}});
            }
        }
    }
}