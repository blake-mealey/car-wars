#pragma once
enum EventType {
	EventType_Collision,
	EventType_Accelerate,
	EventType_Damage
};

template<EventType T>
class Event {
private:
	static constexpr EventType type = T;
	int source;
	int affected;
public:
	Event(int s, int a) : source(s), affected(a) {}
	int GetSource() { return source; }
	int GetAffected() { return affected; }
	static constexpr EventType GetType() { return type; }
	static constexpr unsigned short GetBitMask() { return 1 << T; }
};

class CollisionEvent : public Event<EventType_Collision> {
private:
	int damage;
public:
	CollisionEvent(int s, int a, int d) : Event(s, a), damage(d) {}
	int GetDamage() { return damage; }
};