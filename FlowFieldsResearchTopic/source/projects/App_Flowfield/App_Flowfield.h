#ifndef ASTAR_APPLICATION_H
#define ASTAR_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"
#include "SteeringAgent.h"
#include "SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"


//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_FlowFieldPathfinding final : public IApp
{
public:
	//Constructor & Destructor
	App_FlowFieldPathfinding() = default;
	virtual ~App_FlowFieldPathfinding();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	const bool ALLOW_DIAGONAL_MOVEMENT = true;
	Elite::Vector2 m_StartPosition = Elite::ZeroVector2;
	Elite::Vector2 m_TargetPosition = Elite::ZeroVector2;
	Elite::Vector2 m_WorldBotLeft;
	Elite::Vector2 m_WorldTopRight;

	//Grid datamembers
	static const int COLUMNS = 20;
	static const int ROWS = 20;
	unsigned int m_SizeCell = 5;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;
	std::vector<float> m_CellCosts;
	std::vector<Elite::Vector2> m_FlowField;

	//Agents
	std::vector<SteeringAgent*> m_AgentPointers;
	BlendedSteering* m_pSteeringBehaviour;
	Seek* m_pSeek;
	Flee* m_pFlee;
	void SetObstacleToAvoid(const SteeringAgent* pAgent, const Elite::Vector2& seekTarget);
	
	//Obstacles
	std::vector<Obstacle*> m_Obstacles;


	//Pathfinding datamembers
	int endPathIdx = invalid_node_index;
	std::vector<Elite::GridTerrainNode*> m_vPath;
	bool m_UpdatePath = true;

	//Editor and Visualisation
	Elite::EGraphEditor m_GraphEditor{};
	Elite::EGraphRenderer m_GraphRenderer{};

	//Debug rendering information
	bool m_bDrawGrid = true;
	bool m_bDrawNodeNumbers = false;
	bool m_bDrawConnections = false;
	bool m_bDrawConnectionsCosts = false;
	bool m_bDrawCellCosts = false;
	bool m_bDrawFlowFieldDir = false;
	bool m_StartSelected = true;
	int m_SelectedHeuristic = 4;
	Elite::Heuristic m_pHeuristicFunction = Elite::HeuristicFunctions::Chebyshev;

	//Functions
	void MakeGridGraph();
	void UpdateImGui();

	//C++ make the class non-copyable
	App_FlowFieldPathfinding(const App_FlowFieldPathfinding&) = delete;
	App_FlowFieldPathfinding& operator=(const App_FlowFieldPathfinding&) = delete;
};
#endif