/*****************************************************************//**
 * \file   OsuParser.hpp
 * \brief  A simple header-only library for parsing osu file
 * 
 * \author peter
 * \date   September 2021
 *********************************************************************/
#pragma once
#include <fstream>
#include <filesystem>
#include <exception>
#include <vector>
#include <string>
#include <sstream>
#include <string_view>
#include <array>
#include <optional>
#include <iostream>
#include <cmath> //pow() on GCC need this header

/**
 * @brief Osu editor play field size
 */
namespace PlayField
{
    template<typename T>
    constexpr T xMax = 512;
    template<typename T>
    constexpr T yMax = 384;
    
    constexpr inline std::string_view xMid = "256";
    constexpr inline std::string_view yMid = "192";
}


namespace details {
    static inline auto TrimStr(std::string& s)
    {
        s.erase(0, s.find_first_not_of("\t\n\v\f\r ")); // left trim
        s.erase(s.find_last_not_of("\t\n\v\f\r ") + 1); // right trim
    }

    /**
     * @brief Split a string representing a [key, value] pair
     * @param line The source string
     * @param separator The separator of key and value
     * @return a pair of [key, value], with whitespace trimmed
     */
    static inline auto SplitKeyVal(std::string_view line, char separator = ':')
    {
        if (auto const splitPos = line.find(separator); splitPos != std::string::npos)
        {
            auto const valuePos = line.find_first_not_of(" ", splitPos + 1);
            return std::pair{
                splitPos == 0 ? "" : line.substr(0, line.find_last_not_of(" ", splitPos - 1) + 1),
                valuePos != std::string::npos ? line.substr(valuePos) : ""
            };
        }

        return std::pair<std::string_view, std::string_view>{"", ""};
    }

    template<typename T = std::string>
    static inline auto SplitWords(std::string_view s)
    {
        std::stringstream ss{ s.data() };
        return std::vector(std::istream_iterator<T>{ss}, std::istream_iterator<T>{});
    }

    static inline auto SplitCommaSeperatedString(std::string_view s)
    {
        std::vector<int> result;
        std::stringstream ss{ s.data() };

        for (int i{}; ss >> i; )
        {
            result.push_back(i);
            if (ss.peek() == ',' || ss.peek() == ' ')
                ss.ignore();
        }

        return result;
    }

    template<size_t N>
    static inline auto SplitString(std::string_view str, char delim = ',', bool appendRest = false)
    {
        std::array<std::string_view, N> result;
        size_t startPos = 0, endPos = str.find(delim);
        size_t i = 0;
        while (endPos != std::string::npos && i < N)
        {
            if (appendRest && i + 1 == N)
                result[i] = str.substr(startPos);
            else
                result[i] = str.substr(startPos, endPos - startPos);
            startPos = endPos + 1;
            endPos = str.find(delim, startPos);
            ++i;
        }
        for (; i < N; ++i)
        {
            if (startPos > str.size() - 1)
                result[i] = "";
            else
            {
                result[i] = str.substr(startPos);
                startPos = std::string::npos;
            }
        }
        return result;
    }

    static inline auto SplitString(std::string_view str, char delim = ',')
    {
        std::vector<std::string_view> result;
        size_t startPos = 0, endPos = str.find(delim);
        while (endPos != std::string::npos)
        {
            result.push_back(str.substr(startPos, endPos - startPos));
            startPos = endPos + 1;
            endPos = str.find(delim, startPos);
        }
        result.push_back(str.substr(startPos));
        return result;
    }


    static inline auto IsEmptyLine(std::string const& line)
    {
        return line.empty() || line == "";
    }

    static inline auto GetLine(std::ifstream& file, std::string& line, bool indicateEmptyLine = true)
    {
        while (std::getline(file, line))
        {
            if (line.back() == '\r')
                line = line.substr(0, line.size() - 1);


            if (line.empty())
            {
                if (indicateEmptyLine)
                    return false;
                else
                    continue;
            }

            if (line[0] == '/' && line[1] == '/')
                continue;

            else if (auto const pos = line.find("//") != std::string::npos)
                line = line.substr(0, pos);
            return true;
        }
        return indicateEmptyLine ? false : static_cast<bool>(file);
    }
}


enum class Countdown : int
{
    No = 0,
    Normal = 1,
    Half = 2,
    Double = 3
};

enum class SampleSet
{
    Default,
    Normal,
    Soft,
    Drum
};

enum class Mode
{
    Osu = 0,
    Taiko = 1,
    Catch = 2,
    Mania = 3
};

enum class OverlayPosition
{
    NoChange,
    Below,
    Above
};

/**
 * @brief General information about the beatmap
 * @see https://github.com/ppy/osu-wiki/blob/master/wiki/osu!_File_Formats/Osu_(file_format)/en.md#general
 */
struct General
{
    /**
     * @brief Location of the audio file relative to the current folder
     */
    std::string audioFile;

    /**
     * @brief Milliseconds of silence before the audio starts playing
     */
    int audioLeanIn = 0;

    /**
     * @brief Time in milliseconds when the audio preview should start
     */
    int previewTime = -1;

    /**
     * @brief Speed of the countdown before the first hit object
     */
    Countdown countdown = Countdown::Normal;

    /**
     * @brief Sample set that will be used if timing points do not override it (Normal, Soft, Drum)
     */
    SampleSet sampleSet = SampleSet::Normal;

    /**
     * @brief Multiplier for the threshold in time where hit objects placed close together stack (0–1)
     */
    float stackLeniency = 0.7f;

    /**
     * @brief Game mode
     */
    Mode mode = Mode::Osu;

    /**
     * @brief Whether or not breaks have a letterboxing effect
     */
    bool letterboxInBreaks = false;

    /**
     * @brief Whether or not the storyboard can use the user's skin images
     */
    bool useSkinSprites = false;

    /**
     * @brief Draw order of hit circle overlays compared to hit numbers
     */
    OverlayPosition overlayPosition = OverlayPosition::NoChange;

    /**
     * @brief Preferred skin to use during gameplay
     */
    std::string skinPreference;

    /**
     * @brief Whether or not a warning about flashing colours should be shown at the beginning of the map
     */
    bool epilepsyWarning = false;

    /**
     * @brief Time in beats that the countdown starts before the first hig object
     */
    int countdownOffset = 0;

    /**
     * @brief Whether or not the "N+1" style key layout is used for osu!mania
     */
    bool specialStyle = false;

    /**
     * @brief Whether or not the storyboard allows widescreen viewing
     */
    bool wideScreenStoryboard = false;

    /**
     * @brief Whether or not sound samples will change rate when playing with speed-changing mods
     */
    bool samplesMatchPlaybackRate = 0;

    /**
     * @brief Parse General info from osu file
     */
    General(std::ifstream& file)
    {
        std::string line;
        while (details::GetLine(file, line))
        {
            auto [key, value] = details::SplitKeyVal(line);

            if (key == "AudioFilename")  audioFile = value;
            else if (key == "AudioLeadIn") audioLeanIn = std::stoi(value.data());
            else if (key == "PreviewTime") previewTime = std::stoi(value.data());
            else if (key == "Countdown") countdown = static_cast<Countdown>(std::stoi(value.data()));
            else if (key == "SampleSet")
            {
                if (value == "Soft") sampleSet = SampleSet::Soft;
                else if (value == "Normal") sampleSet = SampleSet::Normal;
                else if (value == "Drum") sampleSet = SampleSet::Drum;
            }
            else if (key == "StackLeniency") stackLeniency = std::stof(value.data());
            else if (key == "Mode") mode = static_cast<Mode>(std::stoi(value.data()));
            else if (key == "LetterboxInBreaks") letterboxInBreaks = std::stoi(value.data());
            else if (key == "UseSkinSprites") useSkinSprites = std::stoi(value.data());
            else if (key == "OverlayPosition")
            {
                if (value == "NoChange") overlayPosition = OverlayPosition::NoChange;
                else if (value == "Below") overlayPosition = OverlayPosition::Below;
                else if (value == "Above") overlayPosition = OverlayPosition::Above;
            }
            else if (key == "SkinPreference") skinPreference = value;
            else if (key == "EpilepsyWarning") epilepsyWarning = std::stoi(value.data());
            else if (key == "CountdownOffset") countdownOffset = std::stoi(value.data());
            else if (key == "SpecialStyle") specialStyle = std::stoi(value.data());
            else if (key == "WidescreenStoryboard") wideScreenStoryboard = std::stoi(value.data());
            else if (key == "samplesMatchPlaybackRate") samplesMatchPlaybackRate = std::stoi(value.data());
        }
    }

    General() = default;
};

/**
 * @brief Saved settings for the beatmap editor
 */
struct Editor
{
    /**
     * @brief Time in milliseconds of bookmarks
     */
    std::vector<int> bookmarks;

    /**
     * @brief Distance snap multiplier
     */
    float distanceSpacing;

    /**
     * @brief Beat snap divisor
     */
    float beatDivisor;

    /**
     * @brief Grid size
     */
    int gridSize;

    /**
     * @brief Scale factor for the object timeline
     */
    float timelineZoom;

    Editor(std::ifstream& file)
    {
        std::string line;

        while(details::GetLine(file, line))
        {
            auto [key, value] = details::SplitKeyVal(line);
            
            if (key == "Bookmarks")             bookmarks = details::SplitCommaSeperatedString(value);
            else if (key == "DistanceSpacing")  distanceSpacing = std::stof(value.data());
            else if (key == "BeatDivisor")      beatDivisor = std::stof(value.data());
            else if (key == "GridSize")         gridSize = std::stoi(value.data());
            else if (key == "TimelineZoom")     timelineZoom = std::stof(value.data());
        }
    }

    Editor() = default;
};



/**
 * @brief Difficulty settings
 */
struct Difficulty
{
    /**
     * @brief HP setting (0–10)
     */
    float HPDrainRate;

    /**
     * @brief CS setting (0–10)
     */
    float circleSize;

    /**
     * @brief OD setting (0–10)
     */
    float overallDifficulty;

    /**
     * @brief AR setting (0–10)
     */
    float approachRate;

    /**
     * @brief Base slider velocity in hecto-osu! pixels per beat
     */
    float sliderMultiplier;

    /**
     * @brief Amount of slider ticks per beat
     */
    int sliderTickRate;

    Difficulty(std::ifstream& file)
    {
        std::string line;
        while (details::GetLine(file, line))
        {
            auto [key, value] = details::SplitKeyVal(line);

            if (key == "HPDrainRate") HPDrainRate = std::stof(value.data());
            else if (key == "CircleSize") circleSize = std::stof(value.data());
            else if (key == "OverallDifficulty") overallDifficulty = std::stof(value.data());
            else if (key == "ApproachRate") approachRate = std::stof(value.data());
            else if (key == "SliderMultiplier") sliderMultiplier = std::stof(value.data());
            else if (key == "SliderTickRate") sliderTickRate = std::stoi(value.data());
        }
    }

    Difficulty() = default;
};

struct Coord
{
    int x{}, y{};
};

struct Circle;
struct Slider;
struct Spinner;
struct Hold;

/**
 * @brief Base class of all hit objects in osu
 * @details Hit object syntax: 
 *      x,y,time,type,hitSound,objectParams,hitSample
*/
struct HitObject
{
    enum class HitSound
    {
        Normal = 0,
        Whistle = 1,
        Finish = 2,
        Clap = 3
    };

    /**
     * @brief Position in osu!pixels (https://osu.ppy.sh/wiki/zh-hk/osupixel) of the object
     */
    int x;

    /**
     * @brief Position in osu!pixels (https://osu.ppy.sh/wiki/zh-hk/osupixel) of the object
     */
    int y;

    /**
     * @brief Time when the object is to be hit, in milliseconds from the beginning of the beatmap's audio.
     */
    int time;

    /**
     * @brief Indicating the hitsound applied to the object
     */
    HitSound hitSound;

    struct HitSample
    {
        int normalSet;
        int additionSet;
        int index;
        int volume;
        std::string filename;

        HitSample() : normalSet{ 0 }, additionSet{ 0 }, index{ 0 }, volume{ 0 } {}

        HitSample(int normalSet, int additionSet, int index, int volume, std::string filename="")
            :normalSet{normalSet},
            additionSet{additionSet},
            index{index},
            volume{ volume },
            filename{ std::move(filename) }
        {
        }

        HitSample(std::string_view colonSeparatedList) : HitSample()
        {
            //Older version allow this to be completely absent
            if (!colonSeparatedList.empty())
            {
                auto const result = details::SplitString<5>(colonSeparatedList, ':');

                normalSet = std::stoi(result[0].data());
                additionSet = std::stoi(result[1].data());
                index = result[2].empty() ? 0 : std::stoi(result[2].data());
                volume = result[3].empty() ? 0 : std::stoi(result[3].data());
                filename = result[4].empty() ? "" : result[4];
            }
        }
    };

    HitSample hitSample;

    enum class Type
    {
        Circle,
        Slider,
        Spinner,
        Hold,
        All
    };

    HitObject(int x, int y, int time, HitSound hitSound, std::string_view hitSample, Type type)
        :x{x}, y{y}, time{time}, hitSound{hitSound}, hitSample{hitSample}, type{type}
    {
    }

    /**
     * @details Hit object syntax :
     *      x, y, time, type, hitSound, objectParams, hitSample
     */
    HitObject(std::array<std::string_view, 7> const& result)
        : HitObject{
            std::stoi(result[0].data()),
            std::stoi(result[1].data()),
            std::stoi(result[2].data()),
            static_cast<HitSound>(std::stoi(result[4].data())),
            result[6],
            GetType(result[3])
        }
    {
    }

    HitObject(std::string_view line) : HitObject(details::SplitString<7>(line))
    {
    }

    
    /**
     * @brief Calculate the distance to another hit object
     * @param anotherObject the other hit object
     */
    [[nodiscard]] auto distanceTo(HitObject const& anotherObject) const
    {
        return sqrt(pow(x - anotherObject.x, 2) + pow(y - anotherObject.y, 2));
    }

    static auto HandleHitObjects(std::ifstream& file)
    {
        std::string line;
        std::vector<std::unique_ptr<HitObject>> objects;
        while (details::GetLine(file, line))
        {
            auto [_, __, ___, typeStr] = details::SplitString<4>(line);
            try
            {
                switch (HitObject::GetType(typeStr))
                {
                    case Type::Circle:
                        objects.emplace_back(std::make_unique<Circle>(line));
                        break;
                    case Type::Slider:
                        objects.emplace_back(std::make_unique<Slider>(line));
                        break;
                    case Type::Spinner:
                        objects.emplace_back(std::make_unique<Spinner>(line));
                        break;
                    case Type::Hold:
                        objects.emplace_back(std::make_unique<Hold>(line));
                        break;
                    default:
                        throw std::runtime_error{ "Hit object type not implemented" };
                }
            }
            catch (...)
            {
                std::cerr << "Parsing Line: " << line << "failed!\n";
            }
        }
        return objects;
    }

    virtual ~HitObject() = default;
protected:

    /*
        Some special bits representing hit object type
         [type] is an 8-bit integer:
             1 Circle
             2 Slider
             8 Spinner
             *bit[2] set -> new combo
    */
    constexpr static inline unsigned CircleBit = 0b1;
    constexpr static inline unsigned SliderBit = 0b1 << 1;
    constexpr static inline unsigned ComboBit = 0b1 << 2;
    constexpr static inline unsigned SpinnerBit = 0b1 << 3;
    constexpr static inline unsigned HoldBit = 0b1 << 7;
    Type type;

    static inline Type GetType(int num)
    {
        if (num & CircleBit)        return Type::Circle;
        else if (num & SliderBit)   return Type::Slider;
        else if (num & SpinnerBit)  return Type::Spinner;
        else if (num & HoldBit)     return Type::Hold;
    }

    static inline Type GetType(std::string_view str)
    {
        return GetType(std::stoi(str.data()));
    }

    static inline bool IsNewCombo(int num) { return num & ComboBit; }

    static inline bool IsNewCombo(std::string_view str) { return IsNewCombo(std::stoi(str.data())); }

    friend struct OsuFile;

private:
        template<Type type> struct ToType;

        template<Type type> using ToType_t = typename ToType<type>::type;
};
template<> struct HitObject::ToType<HitObject::Type::Circle> { using type = Circle; };
template<> struct HitObject::ToType<HitObject::Type::Slider> { using type = Slider; };
template<> struct HitObject::ToType<HitObject::Type::Spinner> { using type = Spinner; };
template<> struct HitObject::ToType<HitObject::Type::Hold> { using type = Hold; };
template<> struct HitObject::ToType<HitObject::Type::All> { using type = HitObject; };


struct Circle final: HitObject
{
    Circle(std::string_view line) : Circle(details::SplitString<6>(line))
    {
    }

    Circle(std::array<std::string_view, 6> const& result) 
        : HitObject{
            std::array<std::string_view, 7>{
                result[0],      //x
                result[1],  //y
                result[2],  //time
                result[3],  //type
                result[4],  //hitsound,
                "",         //object param
                result[5]
            } 
        }
    {}
};

/**
 * @details Slider syntax:
 *      x,y,time,type,hitSound,curveType|curvePoints,slides,length,edgeSounds,edgeSets,hitSample
 */
struct Slider final: HitObject
{
    enum class CurveType
    {
        Bezier='B',
        CatmullRom='C',
        Linear='L',
        Circle='C'
    };

    struct EdgeSet
    {
        int normalSet;
        int additionSet;
        EdgeSet(std::string_view str)
        {
            auto [normalSetStr, additionSetStr] = details::SplitKeyVal(str);
            normalSet = std::stoi(normalSetStr.data());
            additionSet = std::stoi(additionSetStr.data());
        }
    };

    CurveType curveType;
    std::vector<Coord> curvePoints;
    int slides;
    float length;
    std::vector<int> edgeSounds;
    std::vector<EdgeSet> edgeSets;

    Slider(std::string_view line) : Slider(details::SplitString<11>(line))
    {
    }

    Slider(std::array<std::string_view, 11> const& result)
        : HitObject(std::array<std::string_view, 7>{
            result[0],  //x
            result[1],  //y
            result[2],  //time
            result[3],  //type
            result[4],  //hitSound
            "",         //objectParams(unused)
            result[10]
        })
    {
        auto const& sliderParam = result[5];
        
        auto [curveTypeChar, curvePointsStr] = details::SplitKeyVal(sliderParam, '|');
        curveType = static_cast<CurveType>(curveTypeChar.front());
        for (auto const& curvePointStr : details::SplitString(curvePointsStr, '|'))
        {
            auto [x, y] = details::SplitKeyVal(curvePointsStr);
            curvePoints.push_back(Coord{ std::stoi(x.data()), std::stoi(y.data()) });
        }

        slides = std::stoi(result[6].data());
        length = std::stof(result[7].data());

        if (!result[8].empty())
        {
            for (auto const& edgeSoundStr : details::SplitString(result[8], '|'))
                edgeSounds.push_back(std::stoi(edgeSoundStr.data()));
        }

        if (!result[9].empty())
        {
            for (auto const& edgeSetStr : details::SplitString(result[9], '|'))
                edgeSets.push_back(edgeSetStr);
        }
    }
};


struct Spinner final:HitObject
{
    int endTime;
       
    Spinner(std::string_view line) : Spinner(details::SplitString<7>(line))
    {
    }

    /**
     * @details Spinner syntax:
     *      x,y,time,type,hitSound,endTime,hitSample
     */
    Spinner(std::array<std::string_view, 7> const& result)
        : HitObject{ std::array<std::string_view, 7>{
            PlayField::xMid,    //x
            PlayField::yMid,    //y
            result[2],          //time
            result[3],          //type
            result[4],          //hitsound
            "",                 //object param(unused)F
            result[6]           //hitSample
        }}
    {
        endTime = std::stoi(result[5].data());
    }
};

struct Hold final : HitObject
{
    int endTime;

    Hold(std::string_view line) : Hold(details::SplitString<6>(line))
    {
    }

    /**
     * @details Hold syntax:
     *      x,y,time,type,hitSound,endTime:hitSample
     *  @details Hit object syntax:
     *      x,y,time,type,hitSound,objectParams,hitSample
     */

    Hold(int x, int y, int time, HitSound hitSound, int endTime, std::string_view hitSample)
        : HitObject{x, y, time, hitSound, hitSample, Type::Hold }, endTime{endTime}
    {}

    Hold(std::array<std::string_view, 6> const& result)
        : Hold{
            std::stoi(result[0].data()),
            std::stoi(result[1].data()),
            std::stoi(result[2].data()),
            static_cast<HitSound>(std::stoi(result[3].data())),
            {},
            ""
        }
    {
        auto [endTimeStr, hitSampleStr] = details::SplitKeyVal(result.back());
        endTime = std::stoi(endTimeStr.data());
        hitSample = HitSample{ hitSampleStr };
    }
};

struct TimingPoint
{
    /**
     * @brief Start time of the timing section, in milliseconds from the beginning of the beatmap's audio.
     * The end of the timing section is the next timing point's time (or never, if this is the last timing point).
     */
    int time{};

    /**
     * @brief For uninherited timing points, the duration of a beat, in milliseconds.
     * For inherited timing points, a negative inverse slider velocity multiplier, as a percentage.
     *
     * @example For example, -50 would make all sliders in this timing section twice as fast as SliderMultiplier.
     */
    float beatLength{};

    /**
     * @brief Amount of beats in a measure. Inherited timing points ignore this property.
     */
    int meter{};

    /**
     * @brief Default sample set for hit objects (0 = beatmap default, 1 = normal, 2 = soft, 3 = drum).
     */
    SampleSet sampleSet{};

    /**
     * @brief Custom sample index for hit objects. 0 indicates osu!'s default hitsounds.
     */
    int sampleIndex{};

    /**
     * @brief Volume percentage for hit objects.
     */
    int volume{};

    /**
     * @brief Whether or not the timing point is uninherited.
     */
    bool uninherited{};

    /**
     * @brief Bit flags that give the timing point extra effects.
     */
    unsigned effects{};

    TimingPoint() = default;

    TimingPoint(std::string_view line) : TimingPoint(details::SplitString<8>(line))
    {
    }

    /**
     * @details Timing point syntax: 
     *      time,beatLength,meter,sampleSet,sampleIndex,volume,uninherited,effects
     */
    TimingPoint(std::array<std::string_view, 8> const& result)
        : time{ std::stoi(result[0].data()) },
        beatLength{ std::stof(result[1].data()) },
        meter{ std::stoi(result[2].data()) },
        sampleSet{ static_cast<SampleSet>(std::stoi(result[3].data())) },
        sampleIndex{ std::stoi(result[4].data()) },
        volume{ std::stoi(result[5].data()) },
        uninherited{ static_cast<bool>(std::stoi(result[6].data())) },
        effects{ static_cast<unsigned>(std::stoul(result[7].data())) }
    {
    }

    static auto HandleTimingPoints(std::ifstream& file)
    {
        std::vector<TimingPoint> timingPoints;
        std::string line;
        while (details::GetLine(file, line))
        {
            timingPoints.emplace_back(line);
        }
        return timingPoints;
    }
};


struct Colors
{
    struct Color
    {
        unsigned char r, g, b;

        Color(std::string_view colorString)
        {
            auto const result = details::SplitString<3>(colorString);

            r = std::stoi(result[0].data());
            g = std::stoi(result[1].data());
            b = std::stoi(result[2].data());
        }

        Color(unsigned char r, unsigned char g, unsigned char b) : r{ r }, g{ g }, b{ b }{}

        bool operator==(Color const& rhs) const
        {
            return (r == rhs.r) && (g == rhs.g) && (b == rhs.b);
        }
    };

    /**
     * @brief Additive combo colours
     */
    std::vector<Color> comboColor;

    /**
     * @brief Additive slider track colour
     */
    std::optional<Color> sliderTrackOverride;

    /**
     * @brief Slider border color
     */
    std::optional<Color> sliderBorder;

    Colors(std::ifstream& file)
    {
        std::string line;

        while (details::GetLine(file, line))
        {
            auto [key, value] = details::SplitKeyVal(line);

            if (key.find("Combo") != std::string::npos) comboColor.emplace_back(std::move(value));
            else if (key == "SliderTrackOverride") sliderTrackOverride = Color{ value };
            else if (key == "SliderBorder") sliderBorder = Color{ value };
        }
    }

    Colors() = default;
};




/**
 * @brief Information used to identify the beatmap
 * @see https://github.com/ppy/osu-wiki/blob/master/wiki/osu!_File_Formats/Osu_(file_format)/en.md#metadata
 */
struct Metadata
{
    /**
     * @brief Difficulty ID
     * @note An osu beatmap relates to 1 difficulty, therefore it's actually 1 difficulty of a certain beatmap set.
     * If not present, the value is -1
     */
    int beatmapId = -1;

    /**
     * @brief Beatmap ID
     * @note If not present, the value is -1
     */
    int beatmapSetId = -1;

    /**
     * @brief Romanised song title
     */
    std::string title;

    /**
     * @brief Song title
     */
    std::string titleUnicode;

    /**
     * @brief Romanised song artist
     */
    std::string artist;

    /**
     * @brief Song artist
     */
    std::string artistUnicode;

    /**
     * @brief Beatmap creator
     */
    std::string creator;

    /**
     * @brief Difficulty name
     */
    std::string version;

    /**
     * @brief Original media the song was produced for
     */
    std::string source;

    /**
     * @brief Search terms
     */
    std::vector<std::string> tags;

    /**
     * @brief Parse Metadata from osu file
     */
    Metadata(std::ifstream& osuFile)
    {
        std::string line;

        while (details::GetLine(osuFile, line))
        {
            auto [key, value] = details::SplitKeyVal(line);

            if (key == "Title")                 title = value;
            else if (key == "TitleUnicode")     titleUnicode = value;
            else if (key == "Artist")           artist = value;
            else if (key == "ArtistUnicode")    artistUnicode = value;
            else if (key == "Creator")          creator = value;
            else if (key == "Version")          version = value;
            else if (key == "Source")           source = value;
            else if (key == "Tags")             tags = details::SplitWords(value);
            else if (key == "BeatmapID")        beatmapId = std::stoi(value.data());
            else if (key == "BeatmapSetID")     beatmapSetId = std::stoi(value.data());
        }
    }

    Metadata() = default;
};

/**
 * @warning Not implemented!
 */
struct Event
{
    /**
     * @brief Type of the event. Some events may be referred to by either a name or a number.
     */
    std::string eventType;

    /**
     * @brief Start time of the event, in milliseconds from the beginning of the beatmap's audio.
     * For events that do not use a start time, the default is 0.
     */
    int startTime;

    /**
     * @brief Extra parameters specific to the event's type
     */
    std::string eventParams;

    Event(std::string_view line) : Event(details::SplitString<3>(line))
    {
    }

    Event(std::array<std::string_view, 3> const& result)
        : eventType{ result[0] },
        startTime{ std::stoi(result[1].data()) },
        eventParams{ result[2] }
    {
    }

    static auto HandleEvents(std::ifstream& file)
    {
        std::vector<Event> events;
        std::string line;
        while (details::GetLine(file, line))
        {
            //events.emplace_back(std::move(line));
        }
        return events;
    }
};

struct OsuFile
{
    General general;
    Editor editor;
    Metadata metaData;
    Difficulty difficulty;
    std::vector<Event> events;
    std::vector<TimingPoint> timingPoints;
    Colors colors;
    std::vector<std::unique_ptr<HitObject>> hitObjects;

    OsuFile(std::ifstream&& file)
    {
        std::string line;
        line.reserve(100);
        while (details::GetLine(file, line, false))
        {
            if (line == "[General]")            general = General{ file };
            else if (line == "[Editor]")        editor = Editor{ file };
            else if (line == "[Metadata]")      metaData = Metadata{ file };
            else if (line == "[Difficulty]")    difficulty = Difficulty{ file };
            else if (line == "[Events]")        events = Event::HandleEvents(file);
            else if (line == "[TimingPoints]")  timingPoints = TimingPoint::HandleTimingPoints(file);
            else if (line == "[Colours]")       colors = Colors{ file };
            else if (line == "[HitObjects]")    hitObjects = HitObject::HandleHitObjects(file);
        }
    }
    
    auto& operator[](size_t index)
    {
        return hitObjects[index];
    }

    auto const& operator[](size_t index) const
    {
        return hitObjects[index];
    }

    /**
     * @brief Get the number of all hit objects
     */
    [[nodiscard]] auto getCount() const { return hitObjects.size(); }

    /**
     * @brief Get the number of a certain type of hit objects
     * @tparam type Hit object type
     */
    template<HitObject::Type type>
    [[nodiscard]] auto getCount() const
    {
        if constexpr (type == HitObject::Type::All)
            return getCount();
        else
            return std::count_if(hitObjects.cbegin(), hitObjects.cend(), [](auto const& object) {return object->type == type; });
    }

    /**
     * @brief Get the bpm from file
     * @details
     *    The bpm is stored in the first line in the [TimingPoints] section, as beatLength in milliseconds
     *    As a result, to get BPM, use 60000 (milliseconds per minute) / this number
     */
    [[nodiscard]] float getBPM() const
    {
        return timingPoints.empty() ? 0.0 : 60'000 / timingPoints.front().beatLength;
    }

private:

    template<typename ObjectContainer, HitObject::Type type, typename HitObjectType>
    class ObjectIterator
    {
    public:
        /*std::iterator<> is deprecated (in C++17) so we define these boilerplate ourselves*/
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename ObjectContainer::iterator::value_type;
        using difference_type = typename ObjectContainer::iterator::difference_type;
        using pointer = value_type*;
        using reference = value_type&;

        ObjectIterator(typename ObjectContainer::iterator iter, typename ObjectContainer::iterator begin, typename ObjectContainer::iterator end) : iter{ iter }, first { std::move(begin) }, last{ std::move(end) }
        {
            if (iter != last)
                advanceIter();
            else
                --(*this);
        }
        auto& operator*() const { return *dynamic_cast<HitObjectType*>(iter->get()); }
        auto* operator->() const { return dynamic_cast<HitObjectType*>(iter->get()); }
        ObjectIterator& operator++()
        {
            advanceIter();
            return *this;
        }
        ObjectIterator& operator--()
        {
            auto const iter_copy = iter;
            while (iter != first && (*iter)->type != type)
                --iter;
            //If iter is already the the first position but the type is not matched, we need to restore it to previous value pointing to the "correct" type of hit object
            if (iter == first && (*iter)->type != type)
                iter = iter_copy;
            return *this;
        }
        bool operator==(ObjectIterator const& rhs) const { return iter == rhs.iter; }
        bool operator!=(ObjectIterator const& rhs) const { return iter != rhs.iter; }
    private:
        typename ObjectContainer::iterator iter;
        typename ObjectContainer::iterator const first;
        typename ObjectContainer::iterator const last;
        
        void advanceIter()
        {
            auto const iter_copy = iter;
            while (iter != last && (*iter)->type != type)
                ++iter;
            //If iter is already at the end position but the type is not matched, we need to restore it to previous value pointing to the "correct" type of hit object
            if (iter == last && (*iter)->type != type)
                iter = iter_copy;
        }
    };

    template<typename ObjectContainer, typename HitObjectType>
    class ObjectIterator<ObjectContainer, HitObject::Type::All, HitObjectType>
    {
    public:
        /*std::iterator<> is deprecated (in C++17) so we define these boilerplate ourselves*/
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename ObjectContainer::iterator::value_type;
        using difference_type = typename ObjectContainer::iterator::difference_type;
        using pointer = value_type*;
        using reference = value_type&;

        ObjectIterator(typename ObjectContainer::iterator iter, typename ObjectContainer::iterator begin, typename ObjectContainer::iterator end) : iter{ iter }, first{ std::move(begin) }, last{ std::move(end) } {}
        HitObject& operator*() const { return *iter->get(); }
        HitObject* operator->() const { return iter->get(); }
        ObjectIterator& operator++()
        {
            if (iter + 1 != last)
                ++iter;
            return *this;
        }
        ObjectIterator& operator--()
        {
            if (iter != first)
                --iter;
            return *this;
        }
        bool operator==(ObjectIterator const& rhs) const { return iter == rhs.iter; }
        bool operator!=(ObjectIterator const& rhs) const { return iter != rhs.iter; }
    private:
        typename ObjectContainer::iterator iter;
        typename ObjectContainer::iterator const first;
        typename ObjectContainer::iterator const last;
    };

public:

    /**
     * @brief Get the iterator pointing to the first specified type of hit object
     * @tparam type Should be one of the enum values in `HitObject::Type`
     */
    template<HitObject::Type type = HitObject::Type::All>
    auto begin()
    {
        return ObjectIterator<decltype(hitObjects), type, typename HitObject::ToType_t<type>>{hitObjects.begin(), hitObjects.begin(), hitObjects.end()};
    }

    /**
     * @brief Get the iterator pointing to the last specified type of hit object
     * @tparam type Should be one of the enum values in `HitObject::Type`
     */
    template<HitObject::Type type = HitObject::Type::All>
    auto end()
    {
        return ObjectIterator<decltype(hitObjects), type, typename HitObject::ToType_t<type>>{hitObjects.end(), hitObjects.begin(), hitObjects.end()};
    }
};
