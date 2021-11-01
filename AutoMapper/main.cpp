#include <fstream>
#include <array>
#include "../OsuParser.hpp"

constexpr inline auto columns = 4;

auto GetBeatPoint()
{
	std::array<std::vector<bool>, columns> beats;
	
	std::array<std::ifstream, columns> beatFile{
		std::ifstream{"beatLed1.txt"},
		std::ifstream{"beatLed2.txt"},
		std::ifstream{"beatLed3.txt"},
		std::ifstream{"beatLed4.txt"},
	};

	std::string line;
	for(int i = 0; i<beatFile.size(); ++i)
	{
		while (std::getline(beatFile[i], line))
		{
			beats[i].push_back(std::stoi(line));
		}
	}

	return beats;
}

auto GetTimePoint()
{
	std::vector<int> timePoint;
	std::ifstream timeFile{ "timeIndex.txt" };

	std::string line;
	while (std::getline(timeFile, line))
	{
		timePoint.push_back(std::stod(line) * 1'000);
	}
	return timePoint;
}

#include <queue>
#include <tuple>
template<size_t N>
auto CombineTrack(std::array<std::vector<std::unique_ptr<HitObject>>, N> && hitObjectPerTrack)
{
	std::vector<std::unique_ptr<HitObject>> hitObject;
	hitObject.reserve(hitObjectPerTrack.front().size() * hitObjectPerTrack.size());

	using HitObjectContainer = std::vector<std::unique_ptr<HitObject>>;
	using Iterator = typename HitObjectContainer::iterator;
	using Pair = std::pair<HitObjectContainer*, Iterator>;

	constexpr auto compare = [](Pair const& p1, Pair const& p2)
	{
		return (*p1.second)->time > (*p2.second)->time;
	};

	std::priority_queue<Pair, std::vector<Pair>, decltype(compare)> queue(compare);

	for (auto& track : hitObjectPerTrack)
		queue.emplace(Pair{ &track, track.begin() });

	while (!queue.empty())
	{
		auto pair = queue.top();
		hitObject.emplace_back(std::move(*(pair.second)));
		queue.pop();
		if (pair.second != pair.first->end() - 1)
			queue.push(Pair{pair.first, pair.second + 1});
	}

	return hitObject;
}

auto GenerateHitObject(std::vector<int> const& timePoints, std::vector<bool> const& beatPoints, int columnIndex)
{
	std::vector<std::unique_ptr<HitObject>> hitObjects;

	auto const size = timePoints.size();
	for (int i = 0; i < size; ++i)
	{
		if (beatPoints[i])
		{
			if (i - 1 >= 0 && beatPoints[i - 1])
			{
				dynamic_cast<Hold*>(hitObjects.back().get())->endTime = timePoints[i];
			}
			else if (i + 1 < size && beatPoints[i + 1])
			{
				hitObjects.emplace_back(Hold::MakeManiaHitObject(columnIndex, columns, timePoints[i], timePoints[i + 1]));
			}
			else
				hitObjects.emplace_back(Circle::MakeManiaHitObject(columnIndex, columns, timePoints[i]));
		}
	}

	return hitObjects;
}

auto ParseBeat(std::vector<int>const& timePoints, std::array<std::vector<bool>, columns> const& beats)
{

	return CombineTrack(std::array{
		GenerateHitObject(timePoints, beats[0], 0),
		GenerateHitObject(timePoints, beats[1], 1),
		GenerateHitObject(timePoints, beats[2], 2),
		GenerateHitObject(timePoints, beats[3], 3),
	});
}

auto ParseBeat(std::vector<int>const& timePoints, std::array<std::vector<bool>, columns> const& beats, int track)
{
	return GenerateHitObject(timePoints, beats[track], track);
}

auto ProcessHitSound(std::vector<std::unique_ptr<HitObject>>& hitObjects, float const beatLength, int const offset)
{
	for (auto& hitObject : hitObjects)
	{
		if (auto const multiple = (hitObject->time - offset) / beatLength; std::abs(multiple - std::roundf(multiple)) <= 0.1)
			hitObject->hitSound = HitObject::HitSound::Clap;
	}
}

int main()
{

	auto timePoints = GetTimePoint();
	auto beats = GetBeatPoint();



	OsuFile osuFile;
	osuFile.general.audioFile = "audio.wav";
	osuFile.general.mode = Mode::Mania;

	osuFile.metaData.artist = "Jay Chou";
	osuFile.metaData.artistUnicode = "ÖÜ½ÜÂ×";
	osuFile.metaData.creator = "szpeter";
	osuFile.metaData.title = "Zui Chi Bi";
	osuFile.metaData.titleUnicode = "×í³à±Ú";
	osuFile.metaData.version = "test";


	osuFile.editor.distanceSpacing = 3.5;
	osuFile.editor.beatDivisor = 4;
	osuFile.editor.gridSize = 4;
	osuFile.editor.timelineZoom = 1.5;

	TimingPoint t;
	t.time = 0.135 * 1'000;
	t.beatLength = (0.500853658536585- 0.317926829268293) * 4 * 1'000;
	t.uninherited = true;
	t.volume = 100;
	t.meter = 4;
	osuFile.timingPoints.push_back(t);

	osuFile.difficulty.circleSize = 4;
	osuFile.difficulty.HPDrainRate = 3;
	osuFile.difficulty.overallDifficulty = 5;
	osuFile.hitObjects = ParseBeat(timePoints, beats);
	//ProcessHitSound(osuFile.hitObjects, t.beatLength, t.time);
	osuFile.save();

	for (int i = 0; i < columns; ++i)
	{
		osuFile.metaData.version = std::string{ "track" } + std::to_string(i);
		osuFile.hitObjects = ParseBeat(timePoints, beats, i);
		osuFile.save();
	}
}
