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

    GLuint GetVao() const;
    size_t GetParticleCount() const;

    glm::vec3 GetInitialScale() const;
    glm::vec3 GetFinalScale() const;

    Texture* GetTexture() const;
    glm::vec4 GetInitialColor() const;
    glm::vec4 GetFinalColor() const;
    float GetEmissiveness() const;

    float GetLifetimeSeconds() const;

    Transform transform;

private:

    void AddParticle(glm::vec3 p, glm::vec3 v);

    void UpdateBuffers();
    void InitializeBuffers();

    glm::vec3 acceleration;
    float initialSpeed;

    glm::vec3 initialScale;
    glm::vec3 finalScale;

    Texture* texture;
    glm::vec4 initialColor;
    glm::vec4 finalColor;
    float emissiveness;

    Time lifetime;
    Time spawnRate;
    Time nextSpawn;

    std::vector<Particle> particles;

    GLuint vao;
    GLuint vbo;
};
