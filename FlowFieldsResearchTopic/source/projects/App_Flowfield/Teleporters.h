#pragma once
#include "framework\EliteMath\EMath.h"
#include "framework\EliteMath\EMathUtilities.h"
#include <utility>

struct TeleporterPair
{
	std::pair<int, int> PositionIndices{-1,-1};
	int Closest{-1};
};