#pragma once
#include <fstream>
#include "OsuParser.hpp"
class JumpGenerater
{

public:
    JumpGenerater(OsuFile& reference, double avgDistance, const char* output = nullptr);
};

