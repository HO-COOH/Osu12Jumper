/*
    PLEASE READ ME!
    
    This is a header-only library for parsing and generating an osu file.
    The hit objects are stored as:
        std::vector<std::variant<...>>
    which makes more sense because you want to process the consecutive hit objects
    So if you want to loop through the objects and process them, you can use a switch statement
        switch(objects.index())
        {
            case 0:
                ...
            case 1:
                ...
        }
    Or use my provided object-specific iterator to process them by one type at a time:
        for(auto iter = file.begin<Type>(); iter != file.end<Type>(); ++iter)
        {...}
     where Type can be <Circle>, <Slider> and <Spinner>
    Or you want polymorphism and you want to treat them as a <HitObject> type, then you simply substitute <Type> -> <All>
    
    I feel guilty of storing each of them as a pointer, something like:
        std::vector<std::unique_ptr<HitObject>>
    So PLEASE DO NOT JUDGE!
*/

#pragma once
#include <fstream>
#include <filesystem>
#include <exception>
#include <future>
#include <variant>
#include <vector>
#include <string>
#include <sstream>
#include <string_view>

using namespace std::literals;

/*Osu editor play field size*/
constexpr int xMax = 512;
constexpr int yMax = 384;

/*Some special bits representing hit object type
     [type] is an 8-bit integer:
         1 Circle
         2 Slider
         8 Spinner
         *bit[2] set -> new combo 
 */
constexpr unsigned CircleBit = 0b1;
constexpr unsigned SliderBit = 0b10;
constexpr unsigned ComboBit = 0b100;
constexpr unsigned SpinnerBit = 0b1000;

static inline auto TrimStr(std::string& s)
{
    s.erase(0, s.find_first_not_of("\t\n\v\f\r ")); // left trim
    s.erase(s.find_last_not_of("\t\n\v\f\r ") + 1); // right trim
}

static inline auto SplitKeyVal(std::string_view line, char separator = ':')
{
    auto const splitPos = line.find(separator);
    if (splitPos != std::string::npos)
    {
        auto const valuePos = line.find_first_not_of(" ", splitPos + 1);
        return std::pair{
            line.substr(0, splitPos),
            valuePos != std::string::npos? line.substr(valuePos) : ""
        };
    }
    
    return std::pair<std::string_view, std::string_view>{"", ""};
}

template<typename T = std::string>
static inline auto SplitString(std::string_view s)
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

//static inline auto GetLine(std::ifstream& file)
//{
//    std::string s;
//    std::getline(file, s);
//
//}


enum class Countdown : int
{
    No = 0,
    Normal = 1,
    Half = 2,
    Double = 3
};

enum class SampleSet
{
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
        while (std::getline(file, line))
        {
            auto [key, value] = SplitKeyVal(line);

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

        while (std::getline(file, line))
        {
            auto [key, value] = SplitKeyVal(line);
            
            if (key == "Bookmarks")             bookmarks = SplitCommaSeperatedString(value);
            else if (key == "DistanceSpacing")  distanceSpacing = std::stof(value.data());
            else if (key == "BeatDivisor")      beatDivisor = std::stof(value.data());
            else if (key == "GridSize")         gridSize = std::stof(value.data());
            else if (key == "TimelineZoom")      timelineZoom = std::stof(value.data());

        }
    }
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
        while (std::getline(file, line))
        {
            auto [key, value] = SplitKeyVal(line);

            if (key == "HPDrainRate") HPDrainRate = std::stof(value.data());
            else if (key == "CircleSize") circleSize = std::stof(value.data());
            else if (key == "OverallDifficulty") overallDifficulty = std::stof(value.data());
            else if (key == "ApproachRate") approachRate = std::stof(value.data());
            else if (key == "SliderMultiplier") sliderMultiplier = std::stof(value.data());
            else if (key == "SliderTickRate") sliderTickRate = std::stof(value.data());
        }
    }
};

struct Coord
{
    int x, y;
    operator std::string() const
    {
        return std::to_string(x) + "," + std::to_string(y);
    }
};

inline std::ostream& operator<<(std::ostream& os, Coord coord)
{
    os << '(' << coord.x << ", " << coord.y << ')';
    return os;
}

/**
 * @brief Base class of all hit objects in osu
*/
struct HitObject
{
    enum HitSound
    {
        Normal = 0,
        Whistle = 1,
        Finish = 2,
        Clap = 3
    };
    Coord coord;
    int time;
    bool combo;
    HitSound sound;

    HitObject(Coord coord, int time, HitSound sound, bool combo=false):coord(coord), time(time), combo(combo), sound(sound){}
    virtual std::string toLine() = 0;
    operator std::string() { return toLine(); }
    [[nodiscard]] float distanceTo(HitObject const& anotherObject) const
    {
        return static_cast<float>(sqrt(pow(coord.x - anotherObject.coord.x, 2) + pow(coord.y - anotherObject.coord.y, 2)));
    }
    virtual ~HitObject() = default;
};
struct Circle final: HitObject
{
    Circle(Coord coord, int time, HitSound sound, bool combo=false):HitObject{coord, time, sound ,combo} {}
    std::string toLine() override
    {
        return std::string(coord) + "," + std::to_string(time) +","+ std::to_string(combo ? 5 : 2) + ",";
    }
    ~Circle() override = default;
};
struct Slider final: HitObject
{
    enum CurveType
    {
        Bezier='B',
        CatmullRom='C',
        Linear='L',
        Circle='C'
    };
    CurveType curveType;
    std::vector<Coord> curvePoint;
    short slides;
    double length;

    Slider(Coord coord, int time, CurveType curve, std::vector<Coord>&& curvePoint, short slides, double length,HitSound sound,bool combo = false)
        :HitObject{ coord, time, sound ,combo },
        curveType{ curve },
        curvePoint{ std::move(curvePoint) },
        slides{ slides },
        length{length}
    {}
    std::string toLine() override
    {
        /*
            Slider syntax:
            x,y,time,type,hitSound,curveType|curvePoints,slides,length,edgeSounds,edgeSets,hitSample
        */
        std::string line;
        line.reserve(200);
        line += coord;
        line += ',';
        line += std::to_string(time) += ',';
        line += std::to_string(combo ? 6 : 2) += ',';
        line += std::to_string(static_cast<int>(sound)) += ',';
        line += static_cast<char>(curveType);
        for (auto const coord : curvePoint)
        {
            line += '|';
            line += std::to_string(coord.x);
            line += ':';
            line += std::to_string(coord.y);
        }
        line += ',';
        line += std::to_string(slides) += ',';
        line += std::to_string(length) += ',';
        return line;
    }
    static CurveType getCurveType(char curveTypeChar)
    {
        switch (curveTypeChar)
        {
        case 'B':
            return CurveType::Bezier;
        case 'C':
            return CurveType::CatmullRom;
        case 'L':
            return CurveType::Linear;
        case 'P':
            return CurveType::Circle;
        default:
            return {};
        }
    }
    ~Slider() override = default;
};

struct Spinner final:HitObject
{
    int endTime;
    Spinner(int time, int endTime, HitSound sound):HitObject{{xMax/2,yMax/2}, time, sound, true},endTime(endTime){}
    /*
        Spinner syntax:
            x,y,time,type,hitSound,endTime,hitSample
    */
    std::string toLine() override
    {
        return std::string(coord) + "," + std::to_string(time) + "," + std::to_string(8) + "," + std::to_string(static_cast<int>(sound)) + "," + std::to_string(endTime) + ",";
    }
    ~Spinner() override = default;
};

/**
 * @brief A dummy type to indicate iterating through all hit objects
*/
struct All{};

class OsuFile
{
    std::vector<std::variant<Circle, Slider, Spinner>> objects;


    void init(std::ifstream& file, bool lazy, size_t count);
public:
    using ObjectContainer = decltype(objects);
    OsuFile(std::filesystem::directory_entry const& file, bool lazy=false, size_t objCount=SIZE_MAX);
    OsuFile(std::ifstream&& file, bool lazy = false, size_t objCount=SIZE_MAX);
    OsuFile const& operator>>(std::ofstream& out);
    OsuFile const& operator>>(std::string_view fileName);

    template<typename ObjectType, typename = std::enable_if_t<std::is_base_of_v<HitObject, ObjectType>>>
    OsuFile& operator<<(ObjectType&& object);

    HitObject& operator[](size_t index)
    {
        return *std::visit([](auto&& obj) {return static_cast<HitObject*>(&obj); }, objects[index]);
    }




    template<typename ObjectType, typename = std::enable_if_t<std::is_base_of_v<HitObject, ObjectType> || std::is_same_v<ObjectType, All>>>
    [[nodiscard]]auto getCount() const
    {
        return std::count_if(objects.cbegin(), objects.cend(), [](auto const& object) {return std::holds_alternative<ObjectType>(object); });
    }



    friend std::ostream& operator<<(std::ostream& os, OsuFile const& osuFile)
    {

        if (!osuFile.objects.empty())
        {
            os << "\tObject count: \t" << osuFile.objects.size() << '\n';
            int circles{}, sliders{}, spinners{};
            for (auto const& object : osuFile.objects)
            {
                switch (object.index())
                {
                case 0:
                    ++circles;
                    break;
                case 1:
                    ++sliders;
                    break;
                case 2:
                    ++spinners;
                    break;
                default:
                    break;
                }
            }
            os << "\t\tCircles:\t" << circles << "\n\t\tSliders:\t" << sliders << "\n\t\tSpinners:\t" << spinners;
        }

        return os; 
    }

    template<typename ObjectType>
    class ObjectIterator
    {
        ObjectContainer::iterator iter;
        OsuFile const& ref;
    public:
        /*std::iterator<> is deprecated (in C++17) so we define these boilerplate ourselves*/
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = ObjectType;
        using difference_type = typename ObjectContainer::iterator::difference_type;
        using pointer = ObjectType*;
        using reference = ObjectType&;

        ObjectIterator(ObjectContainer::iterator iter, OsuFile const& ref):iter(std::move(iter)), ref(ref)
        {
            while (iter != ref.objects.end() && !std::holds_alternative<ObjectType>(*iter))
                ++iter;
        }
        ObjectType& operator*() const { return std::get<ObjectType>(*iter); }
        ObjectType* operator->() const { return &std::get<ObjectType>(*iter); }
        ObjectIterator& operator++()
        {
            do {
                ++iter;
            } while (iter!=ref.objects.end() && !std::holds_alternative<ObjectType>(*iter));
            return *this;
        }
        ObjectIterator& operator--()
        {
            do {
                --iter;
            } while (iter!=ref.objects.begin() && !std::holds_alternative<ObjectType>(*iter));
            return *this;
        }
        bool operator==(ObjectIterator const& rhs) const { return iter == rhs.iter; }
        bool operator!=(ObjectIterator const& rhs) const { return iter != rhs.iter; }
    };


    template<typename ObjectType, typename = std::enable_if_t<std::is_base_of_v<HitObject, ObjectType> || std::is_same_v<ObjectType, All>>>
    auto begin()
    {
        return ObjectIterator<ObjectType>{objects.begin(), *this};
    }
    template<typename ObjectType, typename = std::enable_if_t<std::is_base_of_v<HitObject, ObjectType> || std::is_same_v<ObjectType, All>>>
    auto end()
    {
        return ObjectIterator<ObjectType>{objects.end(), *this};
    }
};


template<>
class OsuFile::ObjectIterator<All>
{
    ObjectContainer::iterator iter;
    OsuFile const& ref;
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = HitObject;
    using difference_type = typename ObjectContainer::iterator::difference_type;
    using pointer = HitObject*;
    using reference = HitObject&;
    ObjectIterator(ObjectContainer::iterator iter, OsuFile const& ref) :iter(std::move(iter)), ref(ref) {}
    HitObject& operator*() const { return *std::visit([](auto&& obj) {return static_cast<HitObject*>(&obj); }, *iter); }
    HitObject* operator->() const { return std::visit([](auto&& obj) {return static_cast<HitObject*>(&obj); }, *iter); }
    ObjectIterator& operator++()
    {
        ++iter;
        return *this;
    }
    ObjectIterator& operator--()
    {
        --iter;
        return *this;
    }
    bool operator==(ObjectIterator const& rhs) const { return iter == rhs.iter; }
    bool operator!=(ObjectIterator const& rhs) const { return iter != rhs.iter; }
};

template<>
[[nodiscard]] inline auto OsuFile::getCount<All>() const
{
    return objects.size();
}

inline void OsuFile::init(std::ifstream& file, bool lazy, size_t count)
{
    std::string line;
    line.reserve(100);
    /*get Title*/

    /*get difficulty name*/

    /*get bpm*/

    /*read hit objects*/
    if(!lazy)
    {
        while(!file.eof())
        {
            std::getline(file, line);
            if(line.find("HitObjects")!=std::string::npos)
            {
                while(std::getline(file, line) && count--)
                {
                    /*
                        x,y,time,type,hitSound, [objectParams], [hitSample]
                        [type] is an 8-bit integer:
                            1 Circle
                            2 Slider
                            8 Spinner
                            *bit[2] set -> new combo 
                        [hitSound]:
                            0 Normal
                            1 Whistle
                            2 Finish
                            3 Clap
                    */
                    
                    int x, y, t, type, sound;
                    sscanf(line.c_str(), "%d,%d,%d,%d,%d", &x, &y, &t, &type, &sound);
                    if(type & CircleBit)    //circles do not have additional [objectParams]
                        objects.emplace_back(Circle{ 
                            {x,y},
                            t,
                            static_cast<HitObject::HitSound>(sound),
                            static_cast<bool>(type & ComboBit)
                        });
                    else if (type & SliderBit)
                    {
                        /*
                            Slider syntax: x,y,time,type,hitSound,curveType|curvePoints,slides,length,edgeSounds,edgeSets,hitSample
                                curveType(char):
                                    B = bézier
                                    C = centripetal catmull-rom
                                    L = linear
                                    P = perfect circle
                                curvePoints(Pipe-separated list of strings):
                                    Points used to construct the slider. Each point is in the format x:y.
                                slides(integer):
                                    Amount of times the player has to follow the slider's curve back-and-forth before the slider is complete.
                                    It can also be interpreted as the repeat count plus one.
                                length (Decimal):
                                    Visual length in osu! pixels of the slider.
                                
                        */
                        char curveType{};
                        sscanf(line.c_str(), "%d,%d,%d,%d,%d,%c", &x, &y, &t, &type, &sound, &curveType);
                        /*read curve points*/
                        std::vector<Coord> curvePoints;
                        size_t const startIndex = line.find('|');
                        size_t const endIndex = line.find(',', startIndex);
                        size_t currentIndex = startIndex;
                        while(currentIndex<=endIndex)
                        {
                            Coord coord;
                            sscanf(line.c_str() + currentIndex+1, "%d:%d", &coord.x, &coord.y);
                            curvePoints.push_back(coord);
                            currentIndex = line.find('|', currentIndex+1);
                        }
                        short slides{};
                        double length{};
                        sscanf(line.c_str() + endIndex, "%d,%lf", &slides, &length);

                        objects.emplace_back(Slider{ 
                            {x,y},
                            t,
                            Slider::getCurveType(curveType),
                            std::move(curvePoints),
                            slides,
                            length,
                            static_cast<HitObject::HitSound>(sound),
                            static_cast<bool>(type & ComboBit)
                        });
                    }
                    else if(type & SpinnerBit)
                    {
                        int endTime;
                        sscanf(line.c_str(), "%d,%d,%d,%d,%d,%d", &x, &y, &t, &type, &sound, &endTime);
                        objects.emplace_back(Spinner{ t, endTime, static_cast<HitObject::HitSound>(sound) });
                    }
                }
                break;
            }
        }
    }
}

inline OsuFile::OsuFile(std::filesystem::directory_entry const& file, bool lazy, size_t objCount)
{
    if (file.exists() && file.path().extension() == ".osu")
    {
        std::ifstream fs{ file };
        if(!fs.is_open())
            throw std::exception{ "File cannot be opened!" };
        init(fs, lazy, objCount);
    }
    else
        throw std::exception{ ("File: "s + file.path().string() + " is not valid!").c_str() };
}



inline OsuFile::OsuFile(std::ifstream&& file, bool lazy, size_t objCount)
{
    if (!file.is_open())
        throw std::exception{ "File cannot be opened!" };
    init(file, lazy, objCount);
}


inline OsuFile const& OsuFile::operator>>(std::ofstream& out)
{
    out << "osu file format v14"
        << "\n[Metadata]"
        << "\n[Difficulty]"
        << "\n[HitObjects]";
    std::copy(begin<All>(), end<All>(), std::ostream_iterator<std::string>{out, "\n"});
    return *this;
}

inline OsuFile const& OsuFile::operator>>(std::string_view fileName)
{
    std::ofstream fs{ fileName };
    if (!fs.is_open())
        throw std::exception{ ("Cannot open "s + fileName.data() + " for writing!").c_str() };
    *this >> fs;
    return *this;
}

template<typename ObjectType, typename>
OsuFile& OsuFile::operator<<(ObjectType&& object)
{
    objects.emplace_back(std::forward<ObjectType>(object));
    return *this;
}




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
        
        while (std::getline(osuFile, line))
        {
            auto [key, value] = SplitKeyVal(line);

            if (key == "Title")                 title = std::move(value);
            else if (key == "TitleUnicode")     titleUnicode = std::move(value);
            else if (key == "Artist")           artist = std::move(value);
            else if (key == "ArtistUnicode")    artistUnicode = std::move(value);
            else if (key == "Creator")          creator = std::move(value);
            else if (key == "Version")          version = std::move(value);
            else if (key == "Source")           source = std::move(value);
            else if (key == "Tags")             tags = SplitString(value);
            else if (key == "BeatmapId")        beatmapId = std::stoi(value.data());
            else if (key == "BeatmapSetId")     beatmapSetId = std::stoi(value.data());
        }
    }
};