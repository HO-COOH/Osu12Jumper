#pragma once
#include "../../OsuParser.hpp"
#include "Mania.Pattern.hpp"
#include <functional>

namespace Mania
{
	class PatternGenerator
	{
	public:
		virtual Pattern generate() = 0;
	protected:

		/**
		 * @brief The last pattern
		 */
		Pattern const previousPattern;

		/**
		 * @brief The hit object to create the pattern for
		 */
		HitObject const& hitObject;

		/**
		 * @brief The originalBeatmap which `hitObject` is a part of
		 */
		OsuFile const& originalBeatmap;

		int const totalColumns;

		int const randomStart;


		PatternGenerator(
			Pattern&& previousPattern,
			HitObject const& hitObject,
			OsuFile const& beatmap,
			int totalColumns
		) : previousPattern(std::move(previousPattern)),
			hitObject(hitObject),
			originalBeatmap(beatmap),
			totalColumns(totalColumns),
			randomStart(totalColumns == 8? 1:0)
		{}

		/**
		 * @brief Generates a count of notes to be generated from probabilities.
		 * @param p2 Probability for 2 notes to be generated.
		 * @param p3 Probability for 3 notes to be generated.
		 * @param p4 Probability for 4 notes to be generated.
		 * @param p5 Probability for 5 notes to be generated.
		 * @param p6 Probability for 6 notes to be generated.
		 */
		static int GetRandomNoteCount(double p2, double p3, double p4 = 0, double p5 = 0, double p6 = 0);

		double getConversionDifficulty() const;

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
			std::vector<Pattern const*> patterns
		) const;

		int findAvailableColumn(int initialColumn, std::vector<Pattern const*> patterns) const;

		/**
		 * @brief Returns a random column index in the range [lowerBound, upperBound].
		 */
		int getRandomColumn(std::optional<int> lowerBound, std::optional<int> upperBound) const;

		/**
		 * @brief Returns a random column index in the range [randomStart, totalColumns].
		 */
		int getRandomColumn() const;
	};

}