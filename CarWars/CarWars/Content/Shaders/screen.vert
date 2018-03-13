#version 430

layout(location = 0) in vec2 vertexPosition;

uniform mat4 modelMatrix;

out vec2 fragmentUv;

void main() {
	gl_Position = modelMatrix * vec4(vertexPosition, 0.0, 1.0);
	fragmentUv = (vertexPosition + 1.0) / 2.0;
}
