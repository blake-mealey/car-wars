#pragma once
#include "../Entities/EntityManager.h"

template<bool, bool, class E, class C> struct ComponentSelector {
	static void HandleEvent(E *e) { }
};

template<class E, class C> struct ComponentSelector<true, true, E, C> {
	static void HandleEvent(E *e) {
		EntityManager::Components<C>()[EntityManager::FindEntity(e->GetAffected())->GetComponentIndex(C::GetType())].HandleEvent(e);
	}
};

template<class E, class C> struct ComponentSelector<true, false, E, C> {
	static void HandleEvent(E *e) {
		for (C c : EntityManager::Components<C>()) {
			c.HandleEvent(e);
		}
	}
};

template<class E>
class EventHandler {
public:
	static void HandleEvent(E *e) {
#define X(ARG) ComponentSelector <E::GetBitMask() & ARG::GetListeners(), ARG::GetType() < ComponentType_CutOff, E, ARG>::HandleEvent(e);
		COMPONENTS
#undef X
	}
};