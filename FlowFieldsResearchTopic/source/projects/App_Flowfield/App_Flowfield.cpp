//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flowfield.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\FlowField.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"
#include "framework/EliteAI/EliteNavigation/EHeuristicFunctions.h"


using namespace Elite;

//Destructor
App_FlowFieldPathfinding::~App_FlowFieldPathfinding()
{
	SAFE_DELETE(m_pGridGraph);
	for (size_t i = 0; i < m_AgentPointers.size(); i++)
	{
		SAFE_DELETE(m_AgentPointers[i]);
	}
	for (size_t i = 0; i < m_Obstacles.size(); i++)
	{
		SAFE_DELETE(m_Obstacles[i]);
	}
	SAFE_DELETE(m_pSteeringBehaviour);
	SAFE_DELETE(m_pFlee);
	SAFE_DELETE(m_pSeek);
}

//Functions
void App_FlowFieldPathfinding::Start()
{
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));
	//DEBUGRENDERER2D->GetActiveCamera()->SetMoveLocked(true);
	//DEBUGRENDERER2D->GetActiveCamera()->SetZoomLocked(true);

	//Create Graph
	MakeGridGraph();

	m_CellCosts.resize(m_pGridGraph->GetNrOfNodes());
	m_FlowField.resize(m_pGridGraph->GetNrOfNodes());

	endPathIdx = 200;

	//Create Agents
	m_pSeek = new Seek();
	m_pFlee = new Flee();
	
	m_pSteeringBehaviour = new BlendedSteering({ {m_pSeek, 0.8f}, {m_pFlee, 0.2f} });

	m_WorldBotLeft = m_pGridGraph->GetNodeWorldPos(0) - Elite::Vector2{m_pGridGraph->GetCellSize()/2.5f, m_pGridGraph->GetCellSize() / 2.5f };
	m_WorldTopRight = m_pGridGraph->GetNodeWorldPos(m_pGridGraph->GetNrOfNodes()-1) + Elite::Vector2{ m_pGridGraph->GetCellSize() / 2.5f, m_pGridGraph->GetCellSize() / 2.5f };
	for (size_t i = 0; i < 1; i++)
	{
		m_AgentPointers.push_back(new SteeringAgent());
		m_AgentPointers[i]->SetPosition(Elite::Vector2{ Elite::randomFloat(m_WorldBotLeft.x, m_WorldTopRight.x), Elite::randomFloat(m_WorldBotLeft.y, m_WorldTopRight.y) });
		m_AgentPointers[i]->SetSteeringBehavior(m_pSteeringBehaviour);
	}
}

void App_FlowFieldPathfinding::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos
		int closestNode = m_pGridGraph->GetNodeFromWorldPos(mousePos);
		endPathIdx = closestNode;
		m_UpdatePath = true;
	}

	//AGENT UPDATE
	for (SteeringAgent* agent : m_AgentPointers )
	{
		float baseSpeed{ 10.f };
		if (m_pGridGraph->GetNode(m_pGridGraph->GetNodeFromWorldPos(agent->GetPosition()))->GetTerrainType() == TerrainType::Mud)
			agent->SetMaxLinearSpeed(baseSpeed / 3.f);
		else
			agent->SetMaxLinearSpeed(baseSpeed);

		Elite::Vector2 seekTarget{agent->GetPosition() + m_FlowField[m_pGridGraph->GetNodeFromWorldPos(agent->GetPosition())]};
		m_pSeek->SetTarget(seekTarget);
		SetObstacleToAvoid(agent, seekTarget);
		agent->Update(deltaTime);
		agent->TrimToWorld(m_WorldBotLeft, m_WorldTopRight);
	}

	//GRID INPUT
	bool hasGridChanged = m_GraphEditor.UpdateGraph(m_pGridGraph, &m_Obstacles);
	if (hasGridChanged)
	{
		m_UpdatePath = true;
	}

	//IMGUI
	UpdateImGui();


	//CALCULATEPATH
	//If we have nodes and the target is not the startNode, find a path!
	if (m_UpdatePath 
		&& endPathIdx != invalid_node_index)
	{
		//FlowField
		auto flowfield = FlowField<GridTerrainNode, GraphConnection>(m_pGridGraph, Elite::HeuristicFunctions::Manhattan);
		auto endNode = m_pGridGraph->GetNode(endPathIdx);

		//m_vPath = pathfinder.FindPath(startNode, endNode);
		flowfield.CalculateCellCosts(endNode, m_CellCosts);
		flowfield.CreateFlowField(m_CellCosts, m_FlowField, endNode);

		m_UpdatePath = false;
		std::cout << "New Path Calculated" << std::endl;
	}
}

void App_FlowFieldPathfinding::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	m_GraphRenderer.RenderGraph(
		m_pGridGraph, 
		m_bDrawGrid, 
		m_bDrawNodeNumbers, 
		m_bDrawConnections, 
		m_bDrawConnectionsCosts,
		&m_CellCosts,
		m_bDrawCellCosts,
		&m_FlowField,
		m_bDrawFlowFieldDir
	);

	//Render end node on top if applicable
	if (endPathIdx != invalid_node_index)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, { m_pGridGraph->GetNode(endPathIdx) }, END_NODE_COLOR);
	}
	
	//render path below if applicable
	if (m_vPath.size() > 0)
	{
		m_GraphRenderer.RenderHighlightedGrid(m_pGridGraph, m_vPath);
	}
}

void App_FlowFieldPathfinding::SetObstacleToAvoid(const SteeringAgent* pAgent, const Elite::Vector2& seekTarget)
{
	const float avoidanceRadiusSquared{ 70.f };
	auto closestObstacleIt = std::min_element(m_Obstacles.begin(), m_Obstacles.end(), [pAgent](const Obstacle* lh, const Obstacle* rh) {
		return Elite::DistanceSquared(pAgent->GetPosition(), lh->GetCenter()) < Elite::DistanceSquared(pAgent->GetPosition(), rh->GetCenter());
		});
	if (closestObstacleIt != m_Obstacles.end() && Elite::DistanceSquared((*closestObstacleIt)->GetCenter(), pAgent->GetPosition()) < avoidanceRadiusSquared)
	{
		if (Elite::Dot(seekTarget - pAgent->GetPosition(), (*closestObstacleIt)->GetCenter() - pAgent->GetPosition()) > 0.8f)
		{
			std::cout << "avoiding obstacle\n";
			m_pFlee->SetTarget((*closestObstacleIt)->GetCenter());
		}
	}
	m_pFlee->SetTarget(pAgent->GetPosition());
}

void App_FlowFieldPathfinding::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, true, 1.f, 1.5f);
}

void App_FlowFieldPathfinding::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 115;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: target");
		ImGui::Text("RMB: start");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing();ImGui::Separator();ImGui::Spacing();ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("A* Pathfinding");
		ImGui::Spacing();

		ImGui::Text("Middle Mouse");
		ImGui::Text("controls");
		std::string buttonText{""};
		if (m_StartSelected)
			buttonText += "Start Node";
		else
			buttonText += "End Node";

		if (ImGui::Button(buttonText.c_str()))
		{
			m_StartSelected = !m_StartSelected;
		}

		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);
		ImGui::Checkbox("Cell Costs", &m_bDrawCellCosts);
		ImGui::Checkbox("Flow Field Direction", &m_bDrawFlowFieldDir);
		if (ImGui::Combo("", &m_SelectedHeuristic, "Manhattan\0Euclidean\0SqrtEuclidean\0Octile\0Chebyshev", 4))
		{
			switch (m_SelectedHeuristic)
			{
			case 0:
				m_pHeuristicFunction = HeuristicFunctions::Manhattan;
				break;
			case 1:
				m_pHeuristicFunction = HeuristicFunctions::Euclidean;
				break;
			case 2:
				m_pHeuristicFunction = HeuristicFunctions::SqrtEuclidean;
				break;
			case 3:
				m_pHeuristicFunction = HeuristicFunctions::Octile;
				break;
			case 4:
				m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
				break;
			default:
				m_pHeuristicFunction = HeuristicFunctions::Chebyshev;
				break;
			}
		}
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}
