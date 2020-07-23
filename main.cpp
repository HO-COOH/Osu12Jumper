#include <iostream>
#include "OsuParser.hpp"

int main()
{
    OsuFile f{ std::ifstream{"1.osu"} };
    std::cout << f << '\n';
}