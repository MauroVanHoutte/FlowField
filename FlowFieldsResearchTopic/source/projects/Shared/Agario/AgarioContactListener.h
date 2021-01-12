#pragma once
class AgarioAgent;
class AgarioFood;

class AgarioContactListener : public b2ContactListener
{
public:
    AgarioContactListener();

private:
    virtual void BeginContact(b2Contact* contact) override;
    virtual void EndContact(b2Contact* contact) override;

private:
    void OnCollisionPlayerPlayer(AgarioAgent* agentA, AgarioAgent* agentB);
    void OnCollisionFoodPlayer(AgarioAgent* agent, AgarioFood* food);
};

