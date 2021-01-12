#pragma once
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include <vector>

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class FlowField
	{
	public:
		FlowField(GridGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);
		
		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return costSoFar < other.costSoFar;
			};
		};
		void CalculateCellCosts(T_NodeType* pDestinationNode, std::vector<float>& cellCosts);
		void CreateFlowField(const std::vector<float>& cellCosts, std::vector<Vector2>& flowField, const T_NodeType* endNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		GridGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	FlowField<T_NodeType, T_ConnectionType>::FlowField(GridGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void FlowField<T_NodeType, T_ConnectionType>::CalculateCellCosts(T_NodeType* pDestinationNode, std::vector<float>& cellCosts)
	{
		for ( float& cost : cellCosts )
		{
			cost = FLT_MAX;
		}
		NodeRecord startRecord;
		startRecord.pNode = pDestinationNode;
		startRecord.costSoFar = 0;
		std::vector<NodeRecord> openList;
		std::list<T_NodeType*> closedList;
		
		openList.push_back(startRecord);
		closedList.push_back(startRecord.pNode);
		while (!openList.empty())
		{
			auto smallestRecordIt = std::min_element(openList.begin(), openList.end());
			NodeRecord currentRecord = *smallestRecordIt;
			openList[smallestRecordIt - openList.begin()] = openList.back();
			openList.pop_back();
			Vector2 nodePos{ m_pGraph->GetNodePos(currentRecord.pNode) };
			cellCosts[nodePos.y * m_pGraph->GetColumns() + nodePos.x] = currentRecord.costSoFar;
			

			for (T_ConnectionType* con : m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex()))
			{
				NodeRecord newRecord;
				newRecord.pNode = m_pGraph->GetNode(con->GetTo());
				newRecord.pConnection = con;
				newRecord.costSoFar = currentRecord.costSoFar + con->GetCost();

				if (std::find(closedList.begin(), closedList.end(), newRecord.pNode) == closedList.end())
				{
					openList.push_back(newRecord);
					closedList.push_back(newRecord.pNode);
				}
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void FlowField<T_NodeType, T_ConnectionType>::CreateFlowField(const std::vector<float>& cellCosts, std::vector<Vector2>& flowField, const T_NodeType* endNode)
	{
		for (size_t i = 0; i < flowField.size(); i++)
		{
			flowField[i] = ZeroVector2;
		}
		for (auto node : m_pGraph->GetAllNodes() )
		{
			auto connections = m_pGraph->GetNodeConnections(node->GetIndex());
			auto cheapestConnectionIt = std::min_element(connections.begin(), connections.end(), [&cellCosts](const T_ConnectionType* lh, const T_ConnectionType* rh) {
				return cellCosts[lh->GetTo()] < cellCosts[rh->GetTo()];
				});
			if (cheapestConnectionIt == connections.end() || node->GetIndex() == endNode->GetIndex())
			{
				continue;
			}
			flowField[node->GetIndex()] = (m_pGraph->GetNodePos((*cheapestConnectionIt)->GetTo()) - m_pGraph->GetNodePos(node)).GetNormalized();
		}
		flowField[endNode->GetIndex()] = ZeroVector2;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::FlowField<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}