#version 430

layout(location = 0) in vec2 vertexPosition_model;

uniform vec3 cameraRight_world;
uniform vec3 cameraUp_world;
uniform mat4 viewProjectionMatrix;
uniform vec3 billboardPosition;
uniform vec3 billboardScale;

out vec2 fragmentUv;

void main() {
	vec3 vertexPosition_world = billboardPosition +
		0.25 * normalize(cameraRight_world) * vertexPosition_model.x * billboardScale.x +
		0.25 * normalize(cameraUp_world) * vertexPosition_model.y * billboardScale.y;

	gl_Position = viewProjectionMatrix * vec4(vertexPosition_world, 1.0);

	fragmentUv = 0.5 * (vertexPosition_model.xy + vec2(1, 1));
}