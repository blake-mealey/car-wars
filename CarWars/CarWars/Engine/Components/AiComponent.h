#pragma once

#include "Component.h"
#include <json/json.hpp>

enum AiMode {
    AiMode_Waypoints
};

class AiComponent : public Component {
public:
    AiComponent(nlohmann::json data);

    ComponentType GetType() override;
    void HandleEvent(Event* event) override;

    void RenderDebugGui() override;

    void SetTargetEntity(Entity* target);
    Entity* GetTargetEntity() const;
    AiMode GetMode() const;
private:
    AiMode mode;
    Entity *targetEntity;
};

