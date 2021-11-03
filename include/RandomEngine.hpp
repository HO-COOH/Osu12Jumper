#include <random>

constexpr auto PI = 3.1415926535897L;

template<typename T>
auto DegreeToRad(T degree)
{
	constexpr auto coefficient = 180.0 / PI;
	return static_cast<T>(degree / coefficient);
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
	template<typename Float = float>
	static auto getAngleDegree(Float min = 0.0, Float max = 360.0)
	{
		return std::uniform_real_distribution<Float>{min, max}(eng);
	}

	/**
	 * @brief Return a random angle in rad
	 */
	template<typename Float = float>
	static auto getAngleRad(Float min = 0.0, Float max = 2 * PI)
	{
		return getAngleDegree(RadToDegree(min), RadToDegree(max));
	}

	/**
	 * @brief  Return a random integer
	 */
	template<typename T = int>
	static auto getRand(T min, T max)
	{
		if constexpr (std::is_integral_v<T>)
			return std::uniform_int_distribution<T>{min, max}(eng);
		else if constexpr (std::is_floating_point_v<T>)
			return std::uniform_real_distribution<T>{min, max}(eng);
	}

	template<typename T = float, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	static auto getRand()
	{
		return getRand(0.0, 1.0);
	}

	template<typename T = int>
	static auto getRand(Range<T> range)
	{
		return std::uniform_int_distribution<T>{range.min, range.max}(eng);
	}
};