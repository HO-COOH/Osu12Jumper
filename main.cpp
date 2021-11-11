#include <iostream>
#include <string_view>
#include "OsuParser.hpp"
#include <filesystem>
#include <future>
#include "BeatmapConvert/include/BeatmapConvert.hpp"




int main(int argc, char const** argv)
{
	//#ifdef DEBUG
	//	argc = 2;
	//	char arg[] = "./test/Testmapv11.osu";
	//	argv[1] = (char*)malloc(sizeof arg);
	//	strcpy((char*)argv[1], arg);
	//#endif
	if(argc > 1)
	{
		/*convert the specified files*/
		auto const numFileToConvert = argc - 1;
		std::vector<std::future<void>> threads;
		threads.reserve(numFileToConvert);
		for(auto i = 0; i < numFileToConvert; ++i)
		{
			threads.emplace_back(
				std::async(
					std::launch::async,
					[file = argv[i + 1]]()
					{
						if(!std::filesystem::directory_entry{std::filesystem::path{file}}.exists())
						{
							std::cerr << "File: " << file << " does not exist!\n";
							return;
						}
						try{
							/*OsuFile map{ std::ifstream{file} };
							Mania::ManiaBeatmapConverter(map).setTargetColumn(4).convertBeatmap().save();*/
							Mania::ConvertImpl(std::filesystem::directory_entry{ std::filesystem::path{file} }).get();
						}
						catch(...)
						{
							std::cerr << "Cannot convert " << file << '\n';
						}
					}
				)
			);
		}
		
		for (auto&& thread : threads)
			thread.get();
	}
	else
		Mania::ConvertAll("."); //recursively convert all files
}