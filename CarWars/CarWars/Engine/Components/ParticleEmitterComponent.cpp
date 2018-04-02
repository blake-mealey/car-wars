#include "ParticleEmitterComponent.h"
#include "../Systems/StateManager.h"
#include "../Systems/Content/ContentManager.h"

ParticleEmitterComponent::~ParticleEmitterComponent() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

ParticleEmitterComponent::ParticleEmitterComponent(nlohmann::json data) {
    transform = Transform(data);
    texture = ContentManager::GetTexture(data["Texture"]);


//    Particle p;
//    p.velocity = glm::vec3(0.f, 10.f, 0.f);
//    particles.push_back(p);

    AddParticle(glm::vec3(0.f), glm::vec3(0.f, 10.f, 0.f));

//    positions.push_back(glm::vec3(0.f));
//    velocities.push_back(glm::vec3(0.f, 10.f, 0.f));
//    lifetimes.push_back(0.f);

    InitializeBuffers();
}

void ParticleEmitterComponent::UpdateBuffers() {
    glBindBuffer(GL_VERTEX_ARRAY, vbo);
//    glBufferData(vbo, sizeof(Particle) * particles.size(), particles.data(), GL_DYNAMIC_DRAW);
    glBufferData(vbo, sizeof(glm::vec3) * positions.size(), positions.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_VERTEX_ARRAY, 0);
}

void ParticleEmitterComponent::InitializeBuffers() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<const GLvoid*>(0));
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    UpdateBuffers();
}

void ParticleEmitterComponent::Update() {
    const float delta = StateManager::deltaTime.GetSeconds();

    for (size_t i = 0; i < positions.size(); ++i) {
        const glm::vec3 acc = glm::vec3(0.f, -9.81f, 0.f);
        velocities[i] = velocities[i] + delta * acc;
        positions[i] = positions[i] + delta * velocities[i];
        lifetimes[i] += delta;
    }

    AddParticle(glm::vec3(0.f), glm::vec3(0.f, 10.f, 0.f));

    /*for (Particle& particle : particles) {
        const glm::vec3 acc = glm::vec3(0.f, -9.81f, 0.f);
        particle.velocity = particle.velocity + delta * acc;
        particle.position = particle.position + delta * particle.velocity;
        particle.lifetimeSeconds += delta;
    }*/

    UpdateBuffers();
}

Texture* ParticleEmitterComponent::GetTexture() {
    return texture;
}

GLuint ParticleEmitterComponent::GetVao() {
    return vao;
}

size_t ParticleEmitterComponent::GetParticleCount() {
//    return particles.size();
    return positions.size();
}

void ParticleEmitterComponent::AddParticle(glm::vec3 p, glm::vec3 v) {
    positions.push_back(p);
    velocities.push_back(v);
    lifetimes.push_back(0.f);
}

ComponentType ParticleEmitterComponent::GetType() {
    return ComponentType_ParticleEmitter;
}


void ParticleEmitterComponent::HandleEvent(Event* event) { }

void ParticleEmitterComponent::SetEntity(Entity* _entity) {
    Component::SetEntity(_entity);
    transform.parent = &_entity->transform;
}
