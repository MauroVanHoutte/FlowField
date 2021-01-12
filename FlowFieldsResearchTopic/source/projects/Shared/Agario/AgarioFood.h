#pragma once
class AgarioFood
{
public:
	AgarioFood(Elite::Vector2 pos);
	virtual ~AgarioFood();

	virtual void Update(float dt);
	virtual void Render(float dt);

	void MarkForDestroy();
	bool CanBeDestroyed();
	Elite::Vector2 GetPosition() { return m_Position; }

private:
	//--Datamemebers--
	static const float m_Radius;
	
	Elite::Vector2 m_Position;
	Elite::Color m_Color;

	RigidBody* m_pRigidBody = nullptr;
	bool m_ToDestroy = false;
private:
	//C++ make the class non-copyable
	AgarioFood(const AgarioFood&) {};
	AgarioFood& operator=(const AgarioFood&) {};
};

