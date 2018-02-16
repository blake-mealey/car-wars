#version 430

layout (location = 0) in vec3 vertexPosition;

uniform mat4 viewProjectionMatrix;

out vec3 fragmentUv;

void main() {
	fragmentUv = vertexPosition;
	vec4 pos = viewProjectionMatrix * vec4(vertexPosition, 1.0);
	gl_Position = pos.xyww;
}