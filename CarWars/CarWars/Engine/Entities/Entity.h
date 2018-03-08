#pragma once

#include <string>
#include <array>
#include "Transform.h"
#include "../Components/ComponentType.h"
#include<vector>
#include<unordered_map>
#include<typeindex>

#define typeBit 15
#define activeBit 14
#define idEnd 14
#define infoMask 11

using std::array;
using std::vector;

class EntityManager;

class Entity {
	friend class EntityManager;
private:
	//variables
	short id = SHRT_MAX;
	std::string tag;
	vector<short> children;

	//Store component id's sorted by type
	std::unordered_map<std::type_index, vector<unsigned short>> components;
public:
	//variables
	unsigned short transformID;
	short parentID = SHRT_MAX;

	//functions

	//Constructor
	Entity(short _id);

	//Destructor
	~Entity();

	//Helperfunction
	bool isDynamic();
	static bool isDynamic(short entityID);
	unsigned short GetIndex();
	static unsigned short GetIndex(short entityID);
	Transform& GetTransform();
	
	//Pull up a list of children ids
	vector<short>& GetChildren();

	//void RenderDebugGui();

	//Manage Tag
	bool HasTag(std::string _tag) const { return tag.compare(_tag) == 0; }
	std::string GetTag() const { return tag; }
	void SetTag(std::string _tag) { tag = _tag; }

	template<class T>
	void AddComponent(unsigned short i);

	short GetId() const;

	//Access components
	template<class T>
	const vector<unsigned short>* GetComponentIDs() const;
	template<class T>
	T* GetComponent();
	template<class T>
	vector<T*>* GetComponents();
};

template<class T>
void Entity::AddComponent(unsigned short i) {
	components[typeid(T)].push_back(i);
}

template<class T>
const vector<unsigned short>* Entity::GetComponentIDs() const {
	std::unordered_map<std::type_index, vector<unsigned short>>::const_iterator itr;
	itr = components.find(typeid(T));
	if (itr == components.end())
		return nullptr;
	return &itr->second;
}

/*template<class T>
unsigned short Entity::GetComponentIndex() const {
	std::unordered_map<std::type_index, vector<unsigned short>>::const_iterator itr;
	itr = components.find(typeid(T));
	if (itr == components.end())
		return SHRT_MAX;
	return itr->second[0];
}*/

template<class T>
T* Entity::GetComponent() {
	const vector<unsigned short>* ids = GetComponentIDs<T>();
	if (ids != nullptr)
		return &EntityManager::Components<T>()[(*ids)[0]];
	return nullptr;
}

template<class T>
vector<T*>* Entity::GetComponents() {
	const vector<unsigned short>* ids = GetComponentIDs<T>();
	if (ids == nullptr)
		return nullptr;
	vector<T*> comps;
	for (const unsigned short& i : *ids) {
		comps.push_back(&EntityManager::Components<T>()[i]);
	}
	return &comps;
}
