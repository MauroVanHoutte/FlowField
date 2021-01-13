//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Flowfield.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"
#include "framework/EliteAI/EliteNavigation/EHeuristicFunctions.h"
#include "Teleporters.h"


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
	SAFE_DELETE(m_pFlowfield);
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
	m_pFlowfield = new FlowField<GridTerrainNode, GraphConnection>(m_pGridGraph, Elite::HeuristicFunctions::Manhattan);
	RandomizeTeleporter();
	
	m_CellCosts.resize(m_pGridGraph->GetNrOfNodes());
	m_FlowFieldVectors.resize(m_pGridGraph->GetNrOfNodes());

	endPathIdx = 200;

	//Create Agents
	m_pSeek = new Seek();
	m_pFlee = new Flee();
	
	m_pSteeringBehaviour = new BlendedSteering({ {m_pSeek, 0.8f}, {m_pFlee, 0.2f} });

	m_WorldBotLeft = m_pGridGraph->GetNodeWorldPos(0) - Elite::Vector2{m_pGridGraph->GetCellSize()/2.5f, m_pGridGraph->GetCellSize() / 2.5f };
	m_WorldTopRight = m_pGridGraph->GetNodeWorldPos(m_pGridGraph->GetNrOfNodes()-1) + Elite::Vector2{ m_pGridGraph->GetCellSize() / 2.5f, m_pGridGraph->GetCellSize() / 2.5f };
	for (size_t i = 0; i < 50; i++)
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
		switch (m_TeleporterPair.Closest)
		{
		case 1:
			if (m_pGridGraph->GetNodeFromWorldPos(agent->GetPosition()) == m_TeleporterPair.PositionIndices.second)
			{
				agent->SetPosition(m_pGridGraph->GetNodeWorldPos(m_TeleporterPair.PositionIndices.first));
			}
			break;
		case 2:
			if (m_pGridGraph->GetNodeFromWorldPos(agent->GetPosition()) == m_TeleporterPair.PositionIndices.first)
			{
				agent->SetPosition(m_pGridGraph->GetNodeWorldPos(m_TeleporterPair.PositionIndices.second));
			}
			break;
		default:
			break;
		}
		float baseSpeed{ 10.f };

		if (m_pGridGraph->GetNode(m_pGridGraph->GetNodeFromWorldPos(agent->GetPosition()))->GetTerrainType() == TerrainType::Mud)
			agent->SetMaxLinearSpeed(baseSpeed / 3.f);
		else
			agent->SetMaxLinearSpeed(baseSpeed);
		Elite::Vector2 seekTarget{agent->GetPosition() + m_FlowFieldVectors[m_pGridGraph->GetNodeFromWorldPos(agent->GetPosition())]};
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
	auto endNode = m_pGridGraph->GetNode(endPathIdx);
	if (m_UpdatePath 
		&& endPathIdx != invalid_node_index)
	{
		//FlowField

		//m_vPath = pathfinder.FindPath(startNode, endNode);
		m_pFlowfield->CalculateCellCosts(endNode, m_CellCosts, &m_TeleporterPair);

		m_UpdatePath = false;
		std::cout << "New Path Calculated" << std::endl;
	}
	m_pFlowfield->CreateFlowField(m_CellCosts, m_FlowFieldVectors, endNode, &m_AgentPointers, m_TrafficMultiplier);
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
		&m_FlowFieldVectors,
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

	if (m_bDrawTeleporters)
	{
		DEBUGRENDERER2D->DrawSolidCircle(m_pGridGraph->GetNodeWorldPos(m_TeleporterPair.PositionIndices.first), m_pGridGraph->GetCellSize() / 2.f, { 0.f,0.f }, Color{ 0.5f, 0.f, 0.5f }, -1.f);
		DEBUGRENDERER2D->DrawSolidCircle(m_pGridGraph->GetNodeWorldPos(m_TeleporterPair.PositionIndices.second), m_pGridGraph->GetCellSize() / 2.f, { 0.f,0.f }, Color{ 0.5f, 0.f, 0.5f }, -1.f);
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

void App_FlowFieldPathfinding::RandomizeTeleporter()
{
	m_TeleporterPair.PositionIndices.first  = Elite::randomInt(m_pGridGraph->GetNrOfActiveNodes());
	m_TeleporterPair.PositionIndices.second  = Elite::randomInt(m_pGridGraph->GetNrOfActiveNodes());
	m_TeleporterPair.Closest = -1;
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
		ImGui::Begin("Flowfield", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
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


		ImGui::Checkbox("Grid", &m_bDrawGrid);
		ImGui::Checkbox("NodeNumbers", &m_bDrawNodeNumbers);
		ImGui::Checkbox("Connections", &m_bDrawConnections);
		ImGui::Checkbox("Connections Costs", &m_bDrawConnectionsCosts);
		ImGui::Checkbox("Cell Costs", &m_bDrawCellCosts);
		ImGui::Checkbox("Flow Field Direction", &m_bDrawFlowFieldDir);
		ImGui::Checkbox("Teleporters", &m_bDrawTeleporters);
		ImGui::SliderFloat("Traffic Multiplier", &m_TrafficMultiplier, 0.f, 10.f);
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}
