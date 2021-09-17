/*****************************************************************//**
 * \file   JumpGenerater.h
 * \brief  Generate Random 1-2 Jumps
 * 
 * \author peterwhli
 * \date   September 2021
 *********************************************************************/
#pragma once

#include "OsuParser.hpp"

#include <random>
#include <variant>

constexpr auto PI = 3.1415926535897L;

template<typename T>
auto DegreeToRad(T degree)
{
	constexpr auto coefficient = 180.0 / PI;
	return degree / coefficient;
}

template<typename T>
auto RadToDegree(T rad)
{
	constexpr auto coefficient = 180.0 / PI;
	return rad * coefficient;
}

template<typename T>
struct Range
{
	T min{};
	T max{};

	Range() = default;

	Range(T min, T max) : min{ std::min(min, max) }, max{ std::max(min, max) }
	{

	}
};

class RandomEngine
{
	inline static std::mt19937 eng{ std::random_device{}() };
public:

	/**
	 * @brief Return a random angle in degree
	 */
	static auto getAngleDegree(float min = 0, float max = 360)
	{
		return std::uniform_real_distribution<float>{min, max}(eng);
	}

	/**
	 * @brief Return a random angle in rad
	 */
	static auto getAngleRad(float min = 0, float max = 2 * PI)
	{
		return getAngleDegree(RadToDegree(min), RadToDegree(max));
	}

	/**
	 * @brief  Return a random integer
	 */
	template<typename T = int>
	static auto getRand(T min, T max)
	{
		return std::uniform_int_distribution<T>{min, max}(eng);
	}
	
	template<typename T = int>
	static auto getRand(Range<T> range)
	{
		return std::uniform_int_distribution<T>{range.min, range.max}(eng);
	}
};



class JumpGenerator
{
public:
	JumpGenerator(OsuFile& file) : osuFile{ file } {}


	void generate(int count);

	JumpGenerator& setDistance(int distance);
	JumpGenerator& setDistance(int min, int max);
	JumpGenerator& setDistance(Range<int> range);

	~JumpGenerator();
	
private:
	OsuFile& osuFile;

	std::variant<int, Range<int>> length;

	static Coord getNextNotePos(Coord note, float angle, int distance);
};