#include <iostream>
#include "OsuParser.hpp"

int main()
{
    OsuFile f{ std::ifstream{"1.osu"} };
    for (auto i = 0; i < 10; ++i)
        std::cout << f[i].time << '\n';
}