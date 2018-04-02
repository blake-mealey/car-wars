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

    AddParticle(glm::vec3(0.f), glm::vec3(0.f, 10.f, 0.f));

    InitializeBuffers();
}

void ParticleEmitterComponent::UpdateBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * particles.size(), particles.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleEmitterComponent::InitializeBuffers() {
    glGenBuffers(1, &vbo);
    UpdateBuffers();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<const GLvoid*>(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

float UnitRand() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

void ParticleEmitterComponent::Update() {
    const float delta = StateManager::deltaTime.GetSeconds();

    for (auto it = particles.begin(); it != particles.end();) {
        Particle& particle = *it;
        if (particle.lifetimeSeconds > 2.0) {
            it = particles.erase(it);
        } else {
            const glm::vec3 acc = glm::vec3(0.f, -9.81f, 0.f);
            particle.velocity = particle.velocity + delta * acc;
            particle.position = particle.position + delta * particle.velocity;
            particle.lifetimeSeconds += delta;
            ++it;
        }
    }

    if (StateManager::globalTime - lastSpawn > 0.5) {
        lastSpawn = StateManager::globalTime;
        AddParticle(glm::vec3(0.f), glm::vec3(UnitRand(), 10.f, UnitRand()));
    }
}

Texture* ParticleEmitterComponent::GetTexture() {
    return texture;
}

GLuint ParticleEmitterComponent::GetVao() {
    return vao;
}

size_t ParticleEmitterComponent::GetParticleCount() {
    return particles.size();
}

void ParticleEmitterComponent::AddParticle(glm::vec3 p, glm::vec3 v) {
    Particle particle;
    particle.position = p;
    particle.velocity = v;
    particle.lifetimeSeconds = 0.f;
    particles.push_back(particle);
}

void ParticleEmitterComponent::Sort(glm::vec3 cameraPosition) {
    glm::vec3 localCameraPosition = glm::inverse(transform.GetTransformationMatrix()) * glm::vec4(cameraPosition, 1.f);
    std::sort(particles.begin(), particles.end(), [cameraPosition](const Particle& lhs, const Particle& rhs) -> bool {
        return length(lhs.position - cameraPosition) > length(rhs.position - cameraPosition);
    });
    UpdateBuffers();
}

ComponentType ParticleEmitterComponent::GetType() {
    return ComponentType_ParticleEmitter;
}

void ParticleEmitterComponent::HandleEvent(Event* event) { }

void ParticleEmitterComponent::SetEntity(Entity* _entity) {
    Component::SetEntity(_entity);
    transform.parent = &_entity->transform;
}
