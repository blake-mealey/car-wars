#version 430

layout(location = 0) in float navigationScore;
layout(location = 1) in vec3 vertexPosition_world;

uniform mat4 viewProjectionMatrix;

out vec2 vFragmentUvs;

void main() {
	gl_Position = viewProjectionMatrix * vec4(vertexPosition_world, 1);
	vFragmentUvs = vec2(navigationScore, 0.5);
}
