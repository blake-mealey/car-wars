#include "ParticleSystemComponent.h"
#include "../Systems/StateManager.h"

ParticleSystemComponent::ParticleSystemComponent() {
    currentVbo = 0;
    currentTfb = 1;
    isFirst = true;
    time = 0.0;
    particleTexture = nullptr;
}

ComponentType ParticleSystemComponent::GetType() {
    return ComponentType_ParticleSystem;
}

void ParticleSystemComponent::HandleEvent(Event* event) { }

void ParticleSystemComponent::Render(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos) {
    time += StateManager::deltaTime;

    UpdateParticles();
    RenderParticles(viewProjectionMatrix, cameraPos);

    currentVbo = currentTfb;
    currentTfb = (currentTfb + 1) & 0x1;
}

void ParticleSystemComponent::UpdateParticles() {
    ShaderProgram* updateProgram = shaders[Update];
    glUseProgram(updateProgram->GetId());
    
    updateProgram->LoadUniform(UniformName::Time, time.GetSeconds());
    updateProgram->LoadUniform(UniformName::DeltaTime, StateManager::deltaTime.GetSeconds());

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, randomTexture->textureId);
    updateProgram->LoadUniform(UniformName::RandomTexture, 3);

    glEnable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currentVbo]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[currentTfb]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<const GLvoid*>(4));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<const GLvoid*>(16));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<const GLvoid*>(28));

    glBeginTransformFeedback(GL_POINTS);

    if (isFirst) {
        glDrawArrays(GL_POINTS, 0, 1);
        isFirst = false;
    } else {
        glDrawTransformFeedback(GL_POINTS, transformFeedback[currentVbo]);
    }

    glEndTransformFeedback();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void ParticleSystemComponent::RenderParticles(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos) {
    ShaderProgram* billboardProgram = shaders[Billboard];
    glUseProgram(billboardProgram->GetId());

    billboardProgram->LoadUniform(UniformName::ViewProjectionMatrix, viewProjectionMatrix);
    billboardProgram->LoadUniform(UniformName::CameraPosition, cameraPos);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, particleTexture->textureId);
    billboardProgram->LoadUniform(UniformName::DiffuseTexture, 0);

    glDisable(GL_RASTERIZER_DISCARD);

    glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currentTfb]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<const GLvoid*>(4));

    glDrawTransformFeedback(GL_POINTS, transformFeedback[currentTfb]);
    
    glDisableVertexAttribArray(0);
}
