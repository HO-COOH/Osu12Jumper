/*****************************************************************//**
 * \file   BeatmapConvert.hpp
 * \brief  The logic of converting a beatmap -> another game mode
 * 
 * \author peterwhli
 * \date   September 2021
 *********************************************************************/
#pragma once
#include "OsuParser.hpp"
#include <memory>
#include <functional>
#include <limits>

class BeatmapConverter
{

public:
    BeatmapConverter(OsuFile const& beatmap) : originalBeatmap{ beatmap } {}
protected:
    OsuFile const& originalBeatmap;

    /**
     * @brief Performs the conversion of a Beatmap using this Beatmap Converter.
     */
    virtual OsuFile convertBeatmap();

    /**
     * @brief Performs the conversion of a hit object
     * @note This method is generally executed for all objects in a beatmap
     * @param original The hit object to convert
     */
    virtual std::unique_ptr<HitObject> convertHitObject(HitObject const& original) = 0;

    virtual std::vector<std::unique_ptr<HitObject>> convertHitObjects();
};

namespace Mania
{
    enum class ColumnType
    {
        Even,
        Odd, 
        Special
    };

	struct Pattern
	{
		enum Type : unsigned
		{
            None = 0,

            /// <summary>
            /// Keep the same as last row.
            /// </summary>
            ForceStack = 1,

            /// <summary>
            /// Keep different from last row.
            /// </summary>
            ForceNotStack = 1 << 1,

            /// <summary>
            /// Keep as single note at its original position.
            /// </summary>
            KeepSingle = 1 << 2,

            /// <summary>
            /// Use a lower random value.
            /// </summary>
            LowProbability = 1 << 3,

            /// <summary>
            /// Reserved.
            /// </summary>
            Alternate = 1 << 4,

            /// <summary>
            /// Ignore the repeat count.
            /// </summary>
            ForceSigSlider = 1 << 5,

            /// <summary>
            /// Convert slider to circle.
            /// </summary>
            ForceNotSlider = 1 << 6,

            /// <summary>
            /// Notes gathered together.
            /// </summary>
            Gathered = 1 << 7,
            Mirror = 1 << 8,

            /// <summary>
            /// Change 0 -> 6.
            /// </summary>
            Reverse = 1 << 9,

            /// <summary>
            /// 1 -> 5 -> 1 -> 5 like reverse.
            /// </summary>
            Cycle = 1 << 10,

            /// <summary>
            /// Next note will be at column + 1.
            /// </summary>
            Stair = 1 << 11,

            /// <summary>
            /// Next note will be at column - 1.
            /// </summary>
            ReverseStair = 1 << 12
		};

        int const totalColumn;

        /**
         * @brief 
         */
        std::vector<std::unique_ptr<HitObject>> hitObjects;

        Pattern(int totalColumn) : totalColumn{ totalColumn } {}

        bool colunmHasObject(int column) const;

        int numColumnWithObject() const;

        Pattern& operator+=(std::unique_ptr<HitObject> hitObject);
	};

    class PatternGenerator
    {
    public:
        PatternGenerator(
            std::unique_ptr<HitObject> const& hitObject, 
            OsuFile& beatmap, 
            Pattern const& previousPattern,
            int previousTime,
            Coord previousPosition,
            double density,
            Pattern::Type lastStair,
            OsuFile const& originalBeatmap
        );
        
        /**
         * @brief Generates the patterns, each filled 
         */
        Pattern generate();

    private:
        OsuFile& beatmap;

        OsuFile const& originalBeatmap;

        Pattern const previousPattern;
        
        Pattern::Type stairType;

        Pattern::Type convertType;

        std::unique_ptr<HitObject> const& hitObject;

        int randomStart;

        int const totalColumns;

        void addToPattern(Pattern& pattern, int column);

        /**
         * @brief Whether this hit object can generate a note in the special column.
         */
        bool hasSpecialColumn() const;

        int getNextColumn(int last) const;

    protected:
        /**
         * @brief Generates a count of notes to be generated from probabilities.
         * @param p2 Probability for 2 notes to be generated.
         * @param 
         * @param 
         * @param 
         * @param 
         */
        static int GetRandomNoteCount(float p2, float p3, float p4 = 0, float p5 = 0, float p6 = 0);

        Pattern generateRandomNotes(int noteCount);
        
        /**
         * @brief Generates a count of notes to be generated from a list of probabilities.
         */
        int getRandomNoteCount(double p2, double p3, double p4, double p5) const;

        /**
         * @brief Generates a count of notes to be generated from a list of probabilities.
         * @param centreProbability The probability for a note to be added to the centre column.
         * @param p2 Probability for 2 notes to be generated.
         * @param p3 Probability for 3 notes to be generated.
         * @param addToCentre Whether to add a note to the centre column.
         * @return The amount of notes to be generated. The note to be added to the centre column will NOT be part of this count.
         */
        int getRandomNoteCountMirrored(double centreProbability, double p2, double p3, bool& addToCentre);

        /**
         * @brief Generates a random pattern which has both normal and mirrored notes.
         * @param centreProbability The probability for a note to be added to the centre column.
         * @param p2 Probability for 2 notes to be generated.
         * @param p3 Probability for 3 notes to be generated.
         */
        Pattern generateRandomPatternWithMirrored(double centreProbability, double p2, double p3);

        /**
         * 
         */
        Pattern generateRandomPattern(double p2, double p3, double p4, double p5) const;

        /**
         * @brief Returns a random column index in the range [lowerBound, upperBound].
         */
        int getRandomColumn(std::optional<int> lowerBound, std::optional<int> upperBound) const;

        /**
         * @brief Finds a new column in which a HitObject can be placed.
         * @param initialColumn The initial column to test. This may be returned if it is already a valid column.
         * @param lowerBound The minimum column index. If null, `randomStart` is used.
         * @param upperBound The maximum column index. If null, `totalColumns` is used.
         * @param nextColumn A function to retrieve the next column. If null, a randomisation scheme will be used.
         * @param validator A function to perform additional validation checks to determine if a column is a valid candidate for a HitObject.
         * @param patterns A list of patterns for which the validity of a column should be checked against.
         * 
         * @details A column is not a valid candidate if a `HitObject` occupies the same column in any of the patterns.
         * @returns A column which has passed the `validator` check and for which there are no `HitObjects` in any of
         * `patterns` occupying the same column.
         * @retval -1 If there are no valid candidate columns.
         */
        int findAvailableColumn(
            int initialColumn, 
            std::optional<int> lowerBound, 
            std::optional<int> upperBound,
            std::function<int(int)> nextColumn,
            std::function<bool(int)> validator,
            std::vector<Pattern*> patterns
        );

        double getConversionDifficulty() const;

        int getColumnCount() const;

        
    };

    class ManiaBeatmapConverter : public BeatmapConverter
    {
    public:

        ManiaBeatmapConverter(OsuFile const& originalBeatmap) : BeatmapConverter{ originalBeatmap } {}

        /**
         * @brief Performs the conversion of a hit object
         * @note This method is generally executed for all objects in a beatmap
         * @param original The hit object to convert
         */
        std::unique_ptr<HitObject> convertHitObject(HitObject const& original) override;

        void recordNote(HitObject const& note);

        void recordNote(int time, Coord position);

        void computeDensity(int newNoteTime);

        
    private:
        std::vector<std::unique_ptr<HitObject>> generateConverted(HitObject const& original);
        
        /**
         * @brief Maximum number of previous notes to consider for density calculation.
         */
        constexpr static auto MaxNotesForDensity = 7;


        int lastTime{};
        Coord lastPosition{};
        Pattern::Type lastStair = Pattern::Type::Stair;
        Pattern lastPattern;
        std::vector<int> prevNoteTimes;
        int density = std::numeric_limits<int>::max();
    };
}
