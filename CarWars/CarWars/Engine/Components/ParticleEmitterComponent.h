#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <GL/glew.h>
#include <glm/detail/type_vec3.hpp>
#include "../Systems/Content/Texture.h"
#include "../Entities/Transform.h"
#include "../Systems/Time.h"

#define MAX_PARTICLES 100

struct Particle {
    Particle() : lifetimeSeconds(0.f) {}

    glm::vec3 position;
    glm::vec3 velocity;
    float lifetimeSeconds;
};

class ParticleEmitterComponent : public Component {
public:
    ~ParticleEmitterComponent();
    explicit ParticleEmitterComponent(nlohmann::json data);

    ComponentType GetType() override;
    void HandleEvent(Event* event) override;

    void SetEntity(Entity* _entity) override;

    void Update();
    void Sort(glm::vec3 cameraPosition);

    Texture* GetTexture();
    GLuint GetVao();
    size_t GetParticleCount();

    Transform transform;

private:

    void AddParticle(glm::vec3 p, glm::vec3 v);

    void UpdateBuffers();
    void InitializeBuffers();

    Time lastSpawn;

    std::vector<Particle> particles;

    GLuint vao;
    GLuint vbo;

    Texture* texture;
};
