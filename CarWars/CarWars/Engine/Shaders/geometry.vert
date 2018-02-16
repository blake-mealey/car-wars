#version 430

layout(location = 0) in vec3 vertexPosition_model;
layout(location = 1) in vec2 vertexUv;
layout(location = 2) in vec3 vertexNormal_model;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 depthBiasModelViewProjectionMatrix;

out vec3 fragmentPosition_camera;
out vec3 surfaceNormal_camera;
out vec3 eyeDirection_camera;
out vec2 fragmentUv;
out vec4 shadowCoord;


void main() {
	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition_model, 1);

	vec3 vertexPosition_camera = (viewMatrix * modelMatrix * vec4(vertexPosition_model, 1)).xyz;
	eyeDirection_camera = -vertexPosition_camera;

	fragmentPosition_camera = vertexPosition_camera;
	
	surfaceNormal_camera = (viewMatrix * modelMatrix * vec4(vertexNormal_model, 0)).xyz;

	fragmentUv = vertexUv;

	shadowCoord = depthBiasModelViewProjectionMatrix * vec4(vertexPosition_model, 1);
}
