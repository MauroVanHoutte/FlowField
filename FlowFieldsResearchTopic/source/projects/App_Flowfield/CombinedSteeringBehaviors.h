#pragma once
#include "SteeringBehaviors.h"

class Flock;

//****************
//BLENDED STEERING
class BlendedSteering final: public ISteeringBehavior
{
	struct WeightedBehavior
	{
		ISteeringBehavior* pBehavior = nullptr;
		float weight = 0.f;

		WeightedBehavior(ISteeringBehavior* pBehavior, float weight) :
			pBehavior(pBehavior),
			weight(weight)
		{};
	};

	// necessary to let Imgui sliders access weights
	friend class Flock; 
	friend class App_CombinedSteering;

public:
	BlendedSteering(vector<WeightedBehavior> weightedBehaviors);

	void AddBehaviour(WeightedBehavior weightedBehavior) { m_WeightedBehaviors.push_back(weightedBehavior); }
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	vector<WeightedBehavior> m_WeightedBehaviors = {};
};

//*****************
//PRIORITY STEERING
class PrioritySteering final: public ISteeringBehavior
{
public:
	PrioritySteering(vector<ISteeringBehavior*> priorityBehaviors) 
		:m_PriorityBehaviors(priorityBehaviors) 
	{}

	void AddBehaviour(ISteeringBehavior* pBehavior) { m_PriorityBehaviors.push_back(pBehavior); }
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	vector<ISteeringBehavior*> m_PriorityBehaviors = {};
};