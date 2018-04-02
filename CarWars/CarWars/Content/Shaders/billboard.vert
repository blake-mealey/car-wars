#version 430

layout(location = 0) in vec3 vertexPosition_model;
layout(location = 1) in float lifetime_particle;

out float lifetimes_particle;

void main() {
	gl_Position = vec4(vertexPosition_model, 1.0);
	lifetimes_particle = lifetime_particle;
}