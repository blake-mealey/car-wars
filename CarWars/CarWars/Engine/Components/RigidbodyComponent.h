#pragma once

#include "Component.h"

class RigidbodyComponent : public Component {
public:
    RigidbodyComponent();

	ComponentType GetType();
	void HandleEvent(Event *event);

    void RenderDebugGui() override;
private:

};