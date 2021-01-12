	//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_TargetRef.Position - pAgent->GetPosition(); //Desired Velocity
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //rescale to max speed

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const Elite::Vector2 circleCenter{ pAgent->GetPosition() + (pAgent->GetDirection() * m_Offset) };
	const float changeInAngle{ Elite::randomFloat(ToDegrees(m_AngleChange)) - ToDegrees(m_AngleChange)/2 }; //generating random angle between -m_AngleChange and m_AngleChange 
	m_WanderAngle += changeInAngle;

	m_TargetRef.Position.x = circleCenter.x + cos(ToRadians(m_WanderAngle))*m_Radius; //geting point on the circle from angle
	m_TargetRef.Position.y = circleCenter.y + sin(ToRadians(m_WanderAngle))*m_Radius;

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawCircle(circleCenter, m_Radius, { 1,0,1 }, 0.4f);

	return Seek::CalculateSteering(deltaT, pAgent);
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition(); //Velocity -> target
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= -1; //Opposite direction to target
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //rescale to max speed

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);

	return steering;
}

void Arrive::SetSlowRadius(float radius)
{
	m_SlowRadius = radius;
}

void Arrive::SetTargetRadius(float radius)
{
	m_TargetRadius = radius;
}

//ARRIVE
//******
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	const float distanceToTarget{ (m_Target.Position - pAgent->GetPosition()).Magnitude() };
	
	
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition(); //Desired Velocity
	steering.LinearVelocity.Normalize();

	if (distanceToTarget > m_SlowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); //normal speed outside of slow radius
	}
	else
	{
		const float percentageOfSpeed{ (distanceToTarget - m_TargetRadius) / (m_SlowRadius - m_TargetRadius) }; //slow radius = 100%, target radius = 0% of max speed
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * percentageOfSpeed;
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0,1,0 }, 0.4f);
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, m_SlowRadius, { 0,0,1 }, 0.4f);
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, m_TargetRadius, { 1,0,0 }, 0.4f);
	}
	
	
	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	const Elite::Vector2 agentToTarget{ m_Target.Position - pAgent->GetPosition() };
	const float agentToTargetAngle{ atan2f(agentToTarget.y, agentToTarget.x) };
	float currentAngle{ pAgent->GetOrientation() };
	currentAngle += float(M_PI / 2);
	steering.LinearVelocity.x = cos(currentAngle);
	steering.LinearVelocity.y = sin(currentAngle);
	if (true)
	{

	}
	//agentToTarget.Normalize();
	//steering.AngularVelocity = atan2f(agentToTarget.y, agentToTarget.x) - pAgent->GetOrientation();
	//pAgent->SetRotation(atan2f(agentToTarget.y, agentToTarget.x)+(float(M_PI)/2));


	return steering;
}

//PURSUIT
//*******
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const float distanceToTarget{ (m_Target.Position - pAgent->GetLinearVelocity()).Magnitude() };
	const Elite::Vector2 targetSpeedNormalized{ m_Target.LinearVelocity.GetNormalized() };

	m_Target.Position.x += targetSpeedNormalized.x * distanceToTarget/2; //predicting location of the target based on their linearvelocity
	m_Target.Position.y += targetSpeedNormalized.y * distanceToTarget/2;
	return Seek::CalculateSteering(deltaT, pAgent);
}

//EVADE
//*****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	auto distanceToTarget = DistanceSquared(m_Target.Position, pAgent->GetPosition());

	if (distanceToTarget > m_FleeRadius*m_FleeRadius)
	{
		SteeringOutput output;
		output.IsValid = false;
		return output;
	}
	if (distanceToTarget * distanceToTarget > m_Target.LinearVelocity.MagnitudeSquared())
	{
		m_Target.Position.x += m_Target.LinearVelocity.x; //predicting location of the target based on their linearvelocity
		m_Target.Position.y += m_Target.LinearVelocity.y;
	}
	return Flee::CalculateSteering(deltaT, pAgent);
}