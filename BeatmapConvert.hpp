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
		enum class Type : unsigned
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

        int numColumnWithObject();
	};

    class PatternGenerator
    {
    public:
        PatternGenerator(std::unique_ptr<HitObject> hitObject, OsuFile const& beatmap, Pattern previousPattern);
        
        /**
         * @brief Generates the patterns, each filled 
         */
        std::vector<Pattern> generate();

    private:
        OsuFile const& beatmap;

        Pattern const previousPattern;
        
        Pattern::Type stairType;

        Pattern::Type convertType;

        std::unique_ptr<HitObject> hitObject;

        int randomStart;

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
        int getRandomNoteMirrored(double centreProbability, double p2, double p3, bool& addToCentre);

        /**
         * @brief Returns a random column index in the range [lowerBound, upperBound].
         */
        int getRandomColumn(std::optional<int> lowerBound, std::optional<int> upperBound) const;

        int findAvailableColumn(
            int initialColumn, 
            std::optional<int> lowerBound, 
            std::optional<int> upperBound,
            std::function<int(int, int)> nextColumn,
            std::function<bool(int)> validator
        );

        double getConversionDifficulty() const;
    };
}
