#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <map>
#include <glm/glm.hpp>

const struct UniformName {
	static const char* AmbientColor;

	static const char* ModelMatrix;
	static const char* ViewMatrix;
	static const char* ModelViewProjectionMatrix;

	static const char* MaterialDiffuseColor;
	static const char* MaterialSpecularColor;
	static const char* MaterialSpecularity;
	static const char* MaterialEmissiveness;

	static const char* DiffuseTexture;
	static const char* DiffuseTextureEnabled;
	static const char* UvScale;
	
	static const char* ShadowMap;
	static const char* DepthModelViewProjectionMatrix;
	static const char* DepthBiasModelViewProjectionMatrix;
	
	static const char* SkyboxTexture;
	static const char* SkyboxColor;
    static const char* ViewProjectionMatrix;
    
    static const char* SunTexture;
    static const char* SunSizeRadians;
    static const char* SunDirection;
    static const char* Time;
    
    static const char* ScreenTexture;
    static const char* ImageTexture;
    static const char* BlurOffset;
};

class ShaderProgram {
public:
	ShaderProgram();
	ShaderProgram(GLuint id);

	GLuint GetId() const;

	GLuint GetUniformLocation(const char* name);

    void LoadUniform(const char* name, bool v);
    void LoadUniform(const char* name, int v);
    void LoadUniform(const char* name, float v);
    void LoadUniform(const char* name, glm::vec2 v);
    void LoadUniform(const char* name, glm::vec3 v);
    void LoadUniform(const char* name, glm::vec4 v);
    void LoadUniform(const char* name, glm::mat4 v);

private:
	GLuint programId;
	std::map<std::string, GLuint> uniforms;
};
