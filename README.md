![Linux Build](https://github.com/HO-COOH/Osu12Jumper/actions/workflows/linux.yml/badge.svg)
![Windows Build](https://github.com/HO-COOH/Osu12Jumper/actions/workflows/windows.yml/badge.svg)

# Osu 1-2 Jumper
This projects aims to automatically generate random 1-2 jumps to the beat.


Thanks Sotarks!

## Parser
Parser is implemented as a single header library in `OsuParser.hpp`.

It is tested by 2 version of osu beatmap, `v11` and `v14` respectively.

### Quick Start
The osu parser is coded following the documentation from the [official osu wiki](https://osu.ppy.sh/wiki/en/osu%21_File_Formats/Osu_%28file_format%29).

#### Data Members
These sections are represented by a `struct` respectively.
- General
    ```cpp
    struct General
    {
        std::string audioFile;
        int audioLeadIn;
        int previewTime;
        Countdown countdown;
        SampleSet sampleSet;
        float stackLeniency;
        Mode mode;
        bool letterboxInBreaks;
        bool useSkinSprites;
        OverlayPosition overlayPosition;
        std::string skinPreference;
        bool epilepsyWarning;
        int countdownOffset;
        bool specialStyle;
        bool wideScreenStoryboard;
        bool samplesMatchPlaybackRate;
    };
    ```
- Editor
    ```cpp
    struct Editor
    {
        std::vector<int> bookmarks;
        float distanceSpacing;
        float beatDivisor;
        int gridSize;
        std::optional<float> timelineZoom;
    };
    ```
- Metadata
    ```cpp
    struct Metadata
    {
        std::string title;
        std::string titleUnicode;
        std::string artist;
        std::string artistUnicode;
        std::string creator;
        std::string version;
        std::string source;
        std::vector<std::string> tags;
        int beatmapId;
        int beatmapSetId;
    };
    ```
- Difficulty
    ```cpp
    struct Difficulty
    {
        float HPDrainRate;
        float circleSize;
        float overallDifficulty;
        float approachRate;
        float sliderMultiplier;
        int sliderTickRate;
    };
    ```
- Colors
    ```cpp
    struct Colors
    {
        struct Color
        {
            unsigned char r;
            unsigned char g;
            unsigned char b;
        };

        std::vector<Color> comboColor;
        std::optional<Color> sliderTrackOverride;
        std::optional<Color> sliderBorder;
    };
    ```

These following sections are represented by a `std::vector` of `struct` respectively.
- Events -> `std::vector<Event>`
    ```cpp
    struct Event
    {

    };
    ```
- TimingPoints -> `std::vector<TimingPoint>`
    ```cpp
    struct TimingPoint
    {

    };
    ```
- HitObjects -> `std::vector<std::unique_ptr<HitObject>>`
    ```cpp
    struct HitObject
    {

    };
    
    ```

All of above are stored as a member in an `OsuFile` struct.
```cpp
struct OsuFile
{
    General general;
    Editor editor;
    Metadata metaData;
    Difficulty difficulty;
    Events events;
    std::vector<TimingPoint> timingPoints;
    Colors colors;
    std::vector<std::unique_ptr<HitObject>> hitObjects;
};
```

#### Parsing
- To parse a full `.osu` file, simply pass in the file name or a `std::ifstream`. For example,
```cpp
OsuFile file{ "MyMap.osu" };
std::cout << file.metadata.title;
```

- To partially parse a `.osu` file, it it's one of the `struct` members in `OsuFile`, you do the same with parsing the full `.osu` file, just simply pass in the file name or a `std::ifstream`. For example, to parse `Metadata` only, you do
```cpp
Metadata meta{ "MyMap.osu" };
std::cout << file.metadata.title;
``` 
If it's one of the `std::vector` members, there will be a `static` helper function starts with `Handle...` in each of the `struct` to parse and return it. For example, to parse the `TimingPoint` you do
```cpp
auto timingPoints = TimingPoint::HandleTimingPoint("MyMap.osu");
```

## 1-2 Jump Generator
is in `JumpGenerator.hpp`

## Documentation
Documentation is in `html/index.html`.

## Test
Testing code is in `test/`.
There are also 2 beatmaps to be parsed, that will be automatically copied to the testing executable directory. So you do not need to do anything other than running `cmake .`.


## Build
### Requires
1. [Google Test](https://github.com/google/googletest), I recommend using [vcpkg](https://vcpkg.io/en/index.html) to install the package.
2. C++17 compatible compiler
3. CMake
