#pragma once
#include "projects/App_Steering/SteeringAgent.h"


class AgarioAgent : public SteeringAgent
{
public:
	//--- Constructor & Destructor ---
	AgarioAgent(Elite::Vector2 pos);
	AgarioAgent(Elite::Vector2 pos, Elite::Color color);
	virtual ~AgarioAgent();

	//--- Agent Functions ---
	virtual void Update(float dt) override;
	virtual void Render(float dt) override;

	//-- Agario Functions --
	void MarkForUpgrade(float amountOfFood  = 1.0f );
	void MarkForDestroy();
	bool CanBeDestroyed();
	void SetDecisionMaking(Elite::IDecisionMaking* decisionMakingStructure);
	
	void SetToWander();
	void SetToSeek(Elite::Vector2 seekPos);
private:
	Elite::IDecisionMaking* m_DecisionMaking = nullptr;
	float m_ToUpgrade = 0.0f;
	bool m_ToDestroy = false;
	ISteeringBehavior* m_pWander = nullptr;
	ISteeringBehavior* m_pSeek = nullptr;
private:
	void OnUpgrade(float amountOfFood);
};

