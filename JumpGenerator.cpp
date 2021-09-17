#include "JumpGenerator.hpp"

void JumpGenerator::generate(int count)
{
	osuFile.hitObjects.clear();

	auto const startTime = osuFile.timingPoints.front().time;
	auto const bpm = osuFile.getBPM();
	auto const beatLength = osuFile.timingPoints.front().beatLength;

	for (int i = 0; i < count; ++i)
	{
		auto const time = startTime + i * beatLength;

		auto const one_x = RandomEngine::getRand<int>(PlayField::xMax / 2, PlayField::xMax);
		auto const one_y = RandomEngine::getRand<int>(PlayField::yMax / 2, PlayField::yMax);

		auto const angle = RandomEngine::getAngleDegree(0, 180);
		auto const distance = std::holds_alternative<int>(length) ? std::get<int>(length) : RandomEngine::getRand(std::get<Range<int>>(length));

		auto const two = getNextNotePos(Coord{ one_x, one_y }, angle, distance);


		osuFile.hitObjects.emplace_back(std::make_unique<Circle>(one_x, one_y, time, HitObject::HitSound::Clap, HitObject::HitSample{}));
		osuFile.hitObjects.emplace_back(std::make_unique<Circle>(two.x, two.y, time + beatLength / 2, HitObject::HitSound::Clap, HitObject::HitSample{}));
	}
}

JumpGenerator& JumpGenerator::setDistance(int distance)
{
	length = distance;
	return *this;
}

JumpGenerator& JumpGenerator::setDistance(int min, int max)
{
	return setDistance(Range{ min, max });
}

JumpGenerator& JumpGenerator::setDistance(Range<int> range)
{
	length = range;
	return *this;
}

JumpGenerator::~JumpGenerator()
{
	if (!osuFile.hitObjects.empty())
		osuFile.save();
}

Coord JumpGenerator::getNextNotePos(Coord note, float angle, int distance)
{
	return Coord{
		std::clamp(static_cast<int>(note.x - cosf(DegreeToRad(angle)) * distance), 0, PlayField::xMax),
		std::clamp(static_cast<int>(note.y - sinf(DegreeToRad(angle)) * distance), 0, PlayField::yMax)
	};
}
