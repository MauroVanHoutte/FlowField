//=== General Includes ===
#include "stdafx.h"
#include "EFiniteStateMachine.h"
using namespace Elite;

Elite::FiniteStateMachine::FiniteStateMachine(FSMState* startState, Blackboard* pBlackboard)
    : m_pCurrentState(nullptr),
    m_pBlackboard(pBlackboard)
{
    SetState(startState);
}

Elite::FiniteStateMachine::~FiniteStateMachine()
{
    SAFE_DELETE(m_pBlackboard);
}

void Elite::FiniteStateMachine::AddTransition(FSMState* startState, FSMState* toState, FSMTransition* transition)
{
    auto it = m_Transitions.find(startState);
    if (it == m_Transitions.end())
    {
        m_Transitions[startState] = Transitions();
    }
   
    m_Transitions[startState].push_back(std::make_pair(transition, toState));
}

void Elite::FiniteStateMachine::Update(float deltaTime)
{
    auto it = m_Transitions.find(m_pCurrentState);
    if (it != m_Transitions.end())
    {
        //Since we use a normal for loop to loop over all the transitions
        //the order that you add the transitions is the order of importance
        for (TransitionStatePair& transPair : it->second)
        {
            if (transPair.first->ToTransition(m_pBlackboard))
            {
                SetState(transPair.second);
                break;
            }
        }
    }

    if (m_pCurrentState)
        m_pCurrentState->Update(m_pBlackboard, deltaTime );
}

Blackboard* Elite::FiniteStateMachine::GetBlackboard() const
{
    return m_pBlackboard;
}

void Elite::FiniteStateMachine::SetState(FSMState* newState)
{
    if (m_pCurrentState)
        m_pCurrentState->OnExit(m_pBlackboard);
    m_pCurrentState = newState;
    if (m_pCurrentState)
    {
        std::cout << "Entering state: " << typeid(*m_pCurrentState).name() << std::endl;
        m_pCurrentState->OnEnter(m_pBlackboard);
    }
        
}
