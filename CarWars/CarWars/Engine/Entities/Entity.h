#pragma once
#include <array>
#include "Transform.h"
#include "../Components/ComponentType.h"
#include<vector>

#define typeBit 15
#define activeBit 14
#define idEnd 14
#define infoMask 11
#define COMPONENT_CUTOFF 13

using std::array;
using std::vector;

class EntityManager;

class Entity {
	friend class EntityManager;
private:
	Entity(short _id);
	short id;
	std::string tag;
	short parentID;
	vector<short> children;
public:
	unsigned short transformID;
	array<unsigned short, ComponentType_CutOff> components;

	unsigned short GetIndex();
	void RenderDebugGui();
	bool HasTag(std::string _tag) const { return tag.compare(_tag) == 0; }
	std::string GetTag() const { return tag; }
	void SetTag(std::string _tag) { tag = _tag; }

	short GetId() const;
	void SetComponent(ComponentType t, unsigned short i) { components[t] = i; }
	void SetId(int i) { id = i; }						//Remove This!!!!!
	unsigned short GetComponentIndex(ComponentType t) const;
};
