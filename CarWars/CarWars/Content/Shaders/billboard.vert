#version 430

layout(location = 0) in vec3 vertexPosition_model;

void main() {
	gl_Position = vec4(vertexPosition_model, 1.0);
}