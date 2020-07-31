#include <iostream>
#include <random>
#include "OsuParser.hpp"

template<typename T>
T PI = static_cast<T>(3.1415926535897L);

/**
 * @brief generate a random angle [0~360), then convert to rad
 * @return a number indicating a random angle in rad
*/
static float randomAngle()
{
    static std::mt19937 eng{ std::random_device{}() };
    static std::uniform_real_distribution<float> rdAngle{ 0,360 };
    return rdAngle(eng) / (2 * PI<float>);
}

/**
 * @brief Generate the position of next hit circle
 * @param current the position of current circle
 * @param distance the distance to the next hit circle
 * @return the position of next hit circle
*/
static Coord nextCoord(Coord current, float distance)
{
    const auto angle = randomAngle();
    return { static_cast<int>(current.x + cos(angle) * distance), static_cast<int>(current.y + sin(angle) * distance) };
}

int main()
{
    OsuFile f{ std::ifstream{"1.osu"} };
    for(auto i=f.begin<Circle>(); i!=f.end<Circle>(); ++i)
    {
        std::cout << i->coord;
        if (i->combo)
            std::cout << "New combo!";
        std::cout << '\n';
    }
}