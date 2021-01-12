/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringAgent.h: basic agent using steering behaviors
/*=============================================================================*/
#ifndef STEERING_AGENT_H
#define STEERING_AGENT_H

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "../Shared/BaseAgent.h"
#include "SteeringHelpers.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
class ISteeringBehavior;
class Obstacle;

class SteeringAgent final : public BaseAgent
{
public:
	//--- Constructor & Destructor ---
	SteeringAgent() = default;
	virtual ~SteeringAgent() = default;

	//--- Agent Functions ---
	void Update(float dt) override;
	void Render(float dt) override;

	float GetMaxLinearSpeed() const { return m_MaxLinearSpeed; }
	void SetMaxLinearSpeed(float maxLinSpeed) { m_MaxLinearSpeed = maxLinSpeed; }

	float GetMaxAngularSpeed() const { return m_MaxAngularSpeed; }
	void SetMaxAngularSpeed(float maxAngSpeed) { m_MaxAngularSpeed = maxAngSpeed; }

	bool IsAutoOrienting() const { return m_AutoOrient; }
	void SetAutoOrient(bool autoOrient) { m_AutoOrient = autoOrient; }

	float GetOrientation() const { return GetRotation(); }

	Elite::Vector2 GetDirection() const { return GetLinearVelocity().GetNormalized(); }

	void SetSteeringBehavior(ISteeringBehavior* pBehavior) { m_pSteeringBehavior = pBehavior; }
	ISteeringBehavior* GetSteeringBehavior() const { return m_pSteeringBehavior; }

	void SetRenderBehavior(bool isEnabled) { m_RenderBehavior = isEnabled; }
	bool CanRenderBehavior() const { return m_RenderBehavior; }

	void SetObstacleVector(std::vector<Obstacle*>* obstacles);

private:
	//--- Datamembers ---
	ISteeringBehavior* m_pSteeringBehavior = nullptr;
	std::vector<Obstacle*>* m_pObstacles = nullptr;

	float m_MaxLinearSpeed = 10.f;
	float m_MaxAngularSpeed = 30.f;
	bool m_AutoOrient = false;
	bool m_RenderBehavior = false;
};
#endif