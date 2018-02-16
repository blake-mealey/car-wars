#version 430

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_model;

// Values that stay constant for the whole mesh.
uniform mat4 depthModelViewProjectionMatrix;

void main(){
	gl_Position = depthModelViewProjectionMatrix * vec4(vertexPosition_model, 1);
}
