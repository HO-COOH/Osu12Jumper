/*****************************************************************//**
 * \file   BeatmapConvert.hpp
 * \brief  The logic of converting a originalBeatmap -> another game mode
 * 
 * \author peterwhli
 * \date   September 2021
 *********************************************************************/
#pragma once

#include <memory>
#include <functional>
#include <limits>
#include <optional>
#include "OsuParser.hpp"
#include "Mania.Pattern.hpp"
#include "BeatmapConverter.hpp"
#include "PatternGenerator.hpp"
#include <future>

namespace Mania
{
    enum class ColumnType
    {
        Even,
        Odd, 
        Special
    };

    class HitObjectPatternGenerator : PatternGenerator
    {
    public:
        HitObjectPatternGenerator(
            HitObject const& hitObject, 
            OsuFile& beatmap, 
            Pattern&& previousPattern,
            int previousTime,
            Coord previousPosition,
            double density,
            Pattern::Type lastStair,
            OsuFile const& originalBeatmap,
            int const totalColumns
        );
        
        /**
         * @brief Generates the patterns, each filled 
         */
        Pattern generate() override;

        Pattern::Type stairType{};

    private:
        OsuFile& beatmap;


        Pattern::Type convertType{};

        void addToPattern(Pattern& pattern, int column) const;

        /**
         * @brief Whether this hit object can generate a note in the special column.
         */
        bool hasSpecialColumn() const;

        int getNextColumn(int last) const;

    protected:


        Pattern generateRandomNotes(int noteCount) const;
        
        /**
         * @brief Generates a count of notes to be generated from a list of probabilities.
         * @param p2 Probability for 2 notes to be generated.
         * @param p3 Probability for 3 notes to be generated.
         * @param p4 Probability for 4 notes to be generated.
         * @param p5 Probability for 5 notes to be generated.
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
         * @brief Generates a random pattern.
         * @param p2 Probability for 2 notes to be generated.
         * @param p3 Probability for 3 notes to be generated.
         * @param p4 Probability for 4 notes to be generated.
         * @param p5 Probability for 5 notes to be generated.
         * @return The Pattern containing the hit objects
         */
        Pattern generateRandomPattern(double p2, double p3, double p4, double p5) const;

    };

    class DistanceObjectPatternGenerator : public PatternGenerator
    {
    public:
        virtual Pattern generate() override;

        DistanceObjectPatternGenerator(
            HitObject const& hitObject,
            OsuFile& beatmap,
            Pattern&& previousPattern,
            OsuFile const& originalBeatmap,
            int totalColumns
        );

        int const startTime;
        

        /**
         * @brief Aka. slides in `Slider`
         */
        int const spanCount;

        int const endTime;

        /**
         * @brief Duration of one slides
         * @details (EndTime - StartTime) / SpanCount;
         */
        int const segmentDuration;

        Pattern::Type convertType{};
    protected:
        Pattern generateRandomHoldNotes(int startTime, int noteCount);
        Pattern generateRandomNotes(int startTime, int noteCount);
        Pattern generateStair(int startTime);
        Pattern generateRandomMultipleNotes(int startTime);
        Pattern generateNRandomNotes(int startTime, double p2, double p3, double p4);
        Pattern generateTiledHoldNotes(int startTime);
        Pattern generateHoldAndNormalNotes(int startTime);

    private:

        [[nodiscard]] double getEndTime() const;
        [[nodiscard]] double getSegmentDuration() const;

        /**
         * @brief Add a circle
         */
        void addToPattern(
            Pattern& pattern,
            int columnIndex, 
            int startTime, 
            HitObject::HitSound hitSound = HitObject::HitSound{}, 
            HitObject::HitSample hitSample = HitObject::HitSample{}
        );

        /**
         * @brief Add a Hold
         */
        void addToPattern(
            Pattern& pattern,
            int columnIndex,
            int startTime,
            int endTime,
            HitObject::HitSound hitSound = HitObject::HitSound{},
            HitObject::HitSample hitSample = HitObject::HitSample{}
        );
    };

    class ManiaBeatmapConverter : public BeatmapConverter
    {
    public:

        ManiaBeatmapConverter(OsuFile const& originalBeatmap);


        ManiaBeatmapConverter& setTargetColumn(int target);

        OsuFile convertBeatmap() override;

    protected:

        /**
         * @brief Performs the conversion of a hit object
         * @note This method is generally executed for all objects in a originalBeatmap
         * @param original The hit object to convert
         */
        std::vector<std::unique_ptr<HitObject>> convertHitObject(HitObject const& original) override;

        std::vector<std::unique_ptr<HitObject>> generateConverted(HitObject const& original);

        /**
         * @brief The new converted originalBeatmap
         */
        OsuFile beatmap;
        

    private:

        void recordNote(HitObject const& note);

        void recordNote(int time, Coord position);

        void computeDensity(int newNoteTime);

        int getTargetColumn() const;

        //void cleanUpStackedNotes(std::vector<std::unique_ptr<HitObject>>& result) const;

        

        /**
         * @brief Handle generated new pattern, store it into result vector, 
         * update `lastPattern` and `lastStair` 
         */
        void handleNewPattern(Pattern pattern, Pattern::Type stairType ,std::vector<std::unique_ptr<HitObject>>& result);

        /**
         * @brief Maximum number of previous notes to consider for density calculation.
         */
        constexpr static auto MaxNotesForDensity = 7;


        int lastTime{};

        int targetColumns;

        Coord lastPosition{};

        Pattern::Type lastStair = Pattern::Type::Stair;
        std::optional<Pattern> lastPattern;
        std::vector<int> prevNoteTimes;
        double density = std::numeric_limits<int>::max();
    };

    /**
     * @brief Convert all osu maps in the directory (include sub-directories) in parallel
     * @details The converted maps would be named as "<originalVersion>Converted" and saved under the same directory
     */
    void ConvertAll(std::filesystem::recursive_directory_iterator dir);

     /**
     * @brief Convert all osu maps in the directory in parallel
     * @details The converted maps would be named as "<originalVersion>Converted" and saved under the same directory
     */
    void ConvertAll(std::filesystem::directory_iterator dir);

    /**
     * @brief Convert all osu files in the path, which should indicate a directory in parallel
     * @details The converted maps would be named as "<originalVersion>Converted" and saved under the same directory
     */
    void ConvertAll(std::filesystem::path path);

    /**
     * @brief Convert long section of very low density part of maps to break, using a sliding window algorithm
     * @param beatmap A reference to the beatmap to be processed
     * @param windowSizeInBeats The sliding window sizes in terms of beats (4/4)
     * @param hitObjectCountsThreshold Specifies the minimum number of hitObjects in the window in order to not be convert into breaks
     */
    void AddBreaks(OsuFile& beatmap, int windowSizeInBeats = 6, int hitObjectCountsThreshold = 2);

    /**
     * @brief Remove all 1/4 holds in the beapmap
     * @param beatmap The beatmap to be processed
     */
    void RemoveShortHolds(OsuFile& beatmap);

    [[nodiscard]] std::future<void> ConvertImpl(std::filesystem::directory_entry const& entry);
}
