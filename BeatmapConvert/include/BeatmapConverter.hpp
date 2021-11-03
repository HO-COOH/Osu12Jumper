#pragma once
#include "OsuParser.hpp"
class BeatmapConverter
{
public:
    BeatmapConverter(OsuFile const& beatmap) : originalBeatmap{ beatmap } {}

    /**
     * @brief Performs the conversion of a Beatmap using this Beatmap Converter.
     */
    virtual OsuFile convertBeatmap();
protected:
    OsuFile const& originalBeatmap;

    /**
     * @brief Performs the conversion of a hit object
     * @note This method is generally executed for all objects in a originalBeatmap
     * @param original The hit object to convert
     */
    virtual std::vector<std::unique_ptr<HitObject>> convertHitObject(HitObject const& original) = 0;

    virtual std::vector<std::unique_ptr<HitObject>> convertHitObjects();
};