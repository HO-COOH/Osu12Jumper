![Linux Build](https://github.com/HO-COOH/Osu12Jumper/actions/workflows/linux.yml/badge.svg)
![Windows Build](https://github.com/HO-COOH/Osu12Jumper/actions/workflows/windows.yml/badge.svg)

# Osu 1-2 Jumper
This projects aims to automatically generate random 1-2 jumps to the beat.


Thanks Sotarks!

## Parser
Parser is implemented as a single header library in `OsuParser.hpp`.

It is tested by 2 version of osu beatmap, `v11` and `v14` respectively.

## Documentation
Documentation is in `html/index.html`.

## Test
Testing code is in `test/test.cpp`.
There are also 2 beatmaps to be parsed, that will be automatically copied to the testing executable directory. So you do not need to do anything other than running `cmake .`.


## Build
### Requires
1. [Google Test](https://github.com/google/googletest), I recommend using [vcpkg](https://vcpkg.io/en/index.html) to install the package.
2. C++17 compatible compiler
3. CMake
