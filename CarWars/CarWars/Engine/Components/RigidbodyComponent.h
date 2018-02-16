#pragma once

#include "Component.h"

class RigidbodyComponent : public Component {
public:
    RigidbodyComponent();

	ComponentType GetType();
	void HandleEvent(Event *event);
private:

};