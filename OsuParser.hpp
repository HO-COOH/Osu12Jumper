/*
    This is a header-only library for parsing and generating an osu file.
*/

#pragma once
#include <fstream>
#include <filesystem>
#include <exception>
#include <future>
#include <variant>
#include <vector>
#include <string>

using namespace std::literals;
constexpr int xMax = 512;
constexpr int yMax = 384;
constexpr unsigned ComboBit = 4;

struct Coord
{
    int x, y;
};

/**
 * @brief Base class of all hit objects in osu
*/
struct HitObject
{
    Coord coord;
    int time;
    bool combo;
    HitObject(Coord coord, int time, bool combo=false):coord(coord), time(time), combo(combo){}
    virtual std::string toLine() = 0;
    [[nodiscard]] float distanceTo(HitObject const& anotherObject) const
    {
        return sqrt(pow(coord.x - anotherObject.coord.x, 2) + pow(coord.y - anotherObject.coord.y, 2));
    }
    virtual ~HitObject() = default;
};
struct Circle final: HitObject
{
    Circle(Coord coord, int time, bool combo=false):HitObject{coord, time, combo} {}
    std::string toLine() override
    {
        return {};
    }
    ~Circle() override = default;
};
struct Slider final: HitObject
{
    enum CurveType
    {
        Linear,
        Circle
    };

    Slider(Coord coord, int time, bool combo = false):HitObject{ coord, time, combo }{}
    std::string toLine() override
    {
        return {};
    }
    ~Slider() override = default;
};

struct Spinner final:HitObject
{
    int endTime;
    Spinner(int time, int endTime):HitObject{{xMax/2,yMax/2}, time, true},endTime(endTime){}
    std::string toLine() override
    {
        return {};
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
    int length = 0;
    double interval = 0.0;
    std::string title;
    std::string difficultyName;

    void init(std::ifstream& file, bool lazy);
public:
    OsuFile(std::filesystem::directory_entry const& file, bool lazy=false);
    OsuFile(std::ifstream&& file, bool lazy = false);
    OsuFile const& operator>>(std::ofstream& out) const;
    OsuFile const& operator>>(std::string_view fileName) const;

    template<typename ObjectType, typename = std::enable_if_t<std::is_base_of_v<HitObject, ObjectType>>>
    OsuFile& operator<<(ObjectType&& object);

    std::variant<Circle, Slider, Spinner> operator[](size_t index) const;

    [[nodiscard]]auto getLength() const { return length; }
    [[nodiscard]]auto getBPM() const { return 600000.0 / interval; }
    [[nodiscard]]auto getTitle() const { return title; }
    [[nodiscard]]auto getDifficultyName() const { return difficultyName; }

    friend std::ostream& operator<<(std::ostream& os, OsuFile const& osuFile)
    {
        os << "Title: " << osuFile.title << '[' << osuFile.difficultyName << "]\n";
        os << "\tLength:\t\t" << osuFile.length << " ms\n";
        os << "\tBPM: \t\t" << osuFile.getBPM() << '\n';

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
            os << "\tCircles:\t" << circles << "\n\tSliders:\t" << sliders << "\n\tSpinners:\t" << spinners;
        }

        return os; 
    }

    template<typename ObjectType>
    class ObjectIterator
    {
        std::string& lineRef;
    public:
        ObjectIterator(std::string& ref):lineRef(ref){}
        std::variant<Circle, Slider, Spinner> operator*() const;

    };
    template<typename ObjectType, typename = std::enable_if_t<std::is_base_of_v<HitObject, ObjectType> || std::is_same_v<ObjectType, All>>>
    ObjectIterator<ObjectType> begin();

    template<typename ObjectType, typename = std::enable_if_t<std::is_base_of_v<HitObject, ObjectType> || std::is_same_v<ObjectType, All>>>
    ObjectIterator<ObjectType> end();
};

inline void OsuFile::init(std::ifstream& file, bool lazy)
{
    std::string line;
    line.reserve(100);
    /*get Title*/
    while (file)
    {
        std::getline(file, line);
        if (line.find("Title") != std::string::npos)
        {
            title = std::string{ line.cbegin() + 6, line.cend() };
            break;
        }
    }
    /*get difficulty name*/
    while (file)
    {
        std::getline(file, line);
        if (line.find("Version") != std::string::npos)
        {
            difficultyName = std::string{ line.cbegin() + 8, line.cend() };
            break;
        }
    }
    /*get bpm*/
    while (file)
    {
        std::getline(file, line);
        if (line.find("TimingPoints") != std::string::npos)
        {
            std::getline(file, line);
            auto const numBegin = line.find(',') + 1;
            auto const numEnd = line.find(',', numBegin);
            interval = std::stod(std::string{ &line[numBegin],&line[numEnd] });
            break;
        }
    }
    /*read hit objects*/
    if(!lazy)
    {
        while(file)
        {
            std::getline(file, line);
            if(line.find("HitObjects")!=std::string::npos)
            {
                while(file)
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
                    std::getline(file, line);
                    int x, y, t, type, sound;
                    sscanf(line.c_str(), "%d,%d,%d,%d,%d", &x, &y, &t, &type, &sound);
                    if(type & 1)    //circles do not have additional [objectParams]
                        objects.emplace_back(Circle{ {x,y}, t, static_cast<bool>(type & ComboBit) });
                    else if (type & 2)
                    {
                        /*
                            
                        */
                        objects.emplace_back(Slider{ {x,y}, t, static_cast<bool>(type & ComboBit) });
                    }
                    else if(type & 8)
                    {
                        int endTime;
                        sscanf(line.c_str(), "%d,%d,%d,%d,%d,%d", &x, &y, &t, &type, &sound, &endTime);
                        objects.emplace_back(Spinner{ t, endTime });
                    }
                }
                break;
            }
        }
    }
}

inline OsuFile::OsuFile(std::filesystem::directory_entry const& file, bool lazy)
{
    if (file.exists() && file.path().extension() == ".osu")
    {
        std::ifstream fs{ file };
        if(!fs.is_open())
            throw std::exception{ "File cannot be opened!" };
        init(fs, lazy);
    }
    else
        throw std::exception{ ("File: "s + file.path().string() + " is not valid!").c_str() };
}



inline OsuFile::OsuFile(std::ifstream&& file, bool lazy)
{
    if (!file.is_open())
        throw std::exception{ "File cannot be opened!" };
    init(file, lazy);
}


inline OsuFile const& OsuFile::operator>>(std::ofstream& out) const
{
    out << "osu file format v14\n";
    //TODO: implement
    return *this;
}

inline OsuFile const& OsuFile::operator>>(std::string_view fileName) const
{
    std::ofstream fs{ fileName };
    if (!fs.is_open())
        throw std::exception{ ("Cannot open "s + fileName.data() + " for writing!").c_str() };
    (*this) >> fs;
    return *this;
}


inline std::variant<Circle, Slider, Spinner> OsuFile::operator[](size_t index) const
{
}

inline std::variant<Circle, Slider, Spinner> OsuFile::ObjectIterator::operator*() const
{
}

template<typename ObjectType, typename>
inline OsuFile& OsuFile::operator<<(ObjectType&& object)
{

}
