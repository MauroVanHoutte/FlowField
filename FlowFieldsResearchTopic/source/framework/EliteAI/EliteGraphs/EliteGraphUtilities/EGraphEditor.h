#pragma once
#include "framework\EliteAI\EliteGraphs\EGraphNodeTypes.h"
#include "framework\EliteAI\EliteGraphs\EGraphConnectionTypes.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "projects\App_Flowfield\Obstacle.h"

namespace Elite 
{
	class EGraphEditor final
	{
	public:
		EGraphEditor() = default;
		~EGraphEditor() = default;

		bool UpdateGraph(GridGraph<GridTerrainNode, GraphConnection>* pGraph, std::vector<Obstacle*>* obstacles);
	private:
		int m_SelectedTerrainType = (int)TerrainType::Ground;
		
	};
}


