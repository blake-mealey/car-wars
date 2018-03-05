#version 430

layout(location = 0) in vec3 vertexPosition_world;

uniform mat4 viewProjectionMatrix;

void main() {
	gl_Position = viewProjectionMatrix * vec4(vertexPosition_world, 1);
}
