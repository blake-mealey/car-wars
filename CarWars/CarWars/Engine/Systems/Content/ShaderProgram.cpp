#include "ShaderProgram.h"
#include <iostream>

const char* UniformName::AmbientColor = "ambientColor";

const char* UniformName::ModelMatrix = "modelMatrix";
const char* UniformName::ViewMatrix = "viewMatrix";
const char* UniformName::ModelViewProjectionMatrix = "modelViewProjectionMatrix";

const char* UniformName::MaterialDiffuseColor = "materialDiffuseColor";
const char* UniformName::MaterialSpecularColor = "materialSpecularColor";
const char* UniformName::MaterialSpecularity = "materialSpecularity";
const char* UniformName::MaterialEmissiveness = "materialEmissiveness";

const char* UniformName::DiffuseTexture = "diffuseTexture";
const char* UniformName::DiffuseTextureEnabled = "diffuseTextureEnabled";
const char* UniformName::UvScale = "uvScale";

const char* UniformName::ShadowMap = "shadowMap";
const char* UniformName::DepthModelViewProjectionMatrix = "depthModelViewProjectionMatrix";
const char* UniformName::DepthBiasModelViewProjectionMatrix = "depthBiasModelViewProjectionMatrix";

const char* UniformName::SkyboxTexture = "skybox";
const char* UniformName::SkyboxColor = "colorAdjust";
const char* UniformName::ViewProjectionMatrix = "viewProjectionMatrix";

const char* UniformName::SunTexture = "sun";
const char* UniformName::SunSizeRadians = "sunSizeRadians";
const char* UniformName::SunDirection = "sunDirection";
const char* UniformName::Time = "time";

const char* UniformName::ScreenTexture = "screen";
const char* UniformName::ImageTexture = "image";
const char* UniformName::BlurOffset = "offset";

ShaderProgram::ShaderProgram() {}
ShaderProgram::ShaderProgram(GLuint id) : programId(id) {}

GLuint ShaderProgram::GetId() const {
	return programId;
}

GLuint ShaderProgram::GetUniformLocation(const char* name) {
	const auto it = uniforms.find(name);
	if (it == uniforms.end()) {
		uniforms[name] = glGetUniformLocation(programId, name);
	}
	return it->second;
}

void ShaderProgram::LoadUniform(const char* name, bool v) {
    glUniform1ui(GetUniformLocation(name), v);
}

void ShaderProgram::LoadUniform(const char* name, int v) {
    glUniform1i(GetUniformLocation(name), v);
}

void ShaderProgram::LoadUniform(const char* name, float v) {
    glUniform1f(GetUniformLocation(name), v);
}

void ShaderProgram::LoadUniform(const char* name, glm::vec2 v) {
    glUniform2f(GetUniformLocation(name), v.x, v.y);
}

void ShaderProgram::LoadUniform(const char* name, glm::vec3 v) {
    glUniform3f(GetUniformLocation(name), v.x, v.y, v.z);
}

void ShaderProgram::LoadUniform(const char* name, glm::vec4 v) {
    glUniform4f(GetUniformLocation(name), v.x, v.y, v.z, v.w);
}

void ShaderProgram::LoadUniform(const char* name, glm::mat4 v) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &v[0][0]);
}
