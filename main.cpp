#include <iostream>
#include "OsuParser.hpp"
#include "BeatmapConvert/include/BeatmapConvert.hpp"

int main()
{
	//auto maps = Utils::GetAllBeatmapsFrom(std::filesystem::directory_entry{
	//	std::filesystem::path{ "C:\\Users\\peterwhli\\AppData\\Local\\osu!\\Songs\\59631 3L - Three Magic" }
	//	});

	//std::transform(maps.cbegin(), maps.cend(), std::ostream_iterator<std::string>{std::cout, "\n"}, [](auto const& entry)
	//{
	//	return entry.path().string();
	//});

	//OsuFile f{ std::ifstream{"TestMapv11.osu"} };
	//Mania::ManiaBeatmapConverter converter{ f };

	//auto convertedMap = converter.convertBeatmap();
	//convertedMap.metaData.version = "converted";

	//{
	//	std::cout << "Generate:\n\t"
	//		<< convertedMap.getCount<HitObject::Type::Circle>() << " circles\n"
	//		<< '\t' << convertedMap.getCount<HitObject::Type::Hold>() << " holds \n";
	//}

	//convertedMap.save();

	//Mania::ConvertAll(std::filesystem::directory_iterator{ std::filesystem::path{ "." } });
	Mania::ConvertAll(".");
}