#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform vec3 cameraRight_world;
uniform vec3 cameraUp_world;
uniform mat4 viewProjectionMatrix;
uniform vec3 billboardPosition;
uniform vec3 billboardScale;

out vec2 fragmentUv;

void main() {
	vec3 position = gl_in[0].gl_Position.xyz + billboardPosition.xyz;

	position -= cameraRight_world * billboardScale.x * 0.5;
	position += cameraUp_world * billboardScale.y * 0.5;
	gl_Position = viewProjectionMatrix * vec4(position, 1.0);
	fragmentUv = vec2(0.0, 0.0);
	EmitVertex();

	position -= cameraUp_world * billboardScale.y;
	gl_Position = viewProjectionMatrix * vec4(position, 1.0);
	fragmentUv = vec2(0.0, 1.0);
	EmitVertex();

	position += cameraUp_world * billboardScale.y;
	position += cameraRight_world * billboardScale.x;
	gl_Position = viewProjectionMatrix * vec4(position, 1.0);
	fragmentUv = vec2(1.0, 0.0);
	EmitVertex();

	position -= cameraUp_world * billboardScale.y;
	gl_Position = viewProjectionMatrix * vec4(position, 1.0);
	fragmentUv = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}
