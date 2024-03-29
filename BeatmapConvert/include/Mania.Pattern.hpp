#pragma once
#include "OsuParser.hpp"

namespace Mania
{
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

        Pattern(Pattern const&) = delete;

        Pattern(Pattern&&) = default;

        /**
         * @brief Determine whether the specified column has a hit object
         * @param column The index of the column to check, starts from 0
         */
        bool colunmHasObject(int column) const;

        int numColumnWithObject() const;

        /**
         * @brief Add a hit object to the pattern
         */
        Pattern& operator+=(std::unique_ptr<HitObject>&& hitObject);

        /**
         * @brief Add the hit objects contained in the pattern to this pattern
         */
        Pattern& operator+=(Pattern&& pattern);
    };

    std::ostream& operator<<(std::ostream& os, Pattern::Type type);

}
