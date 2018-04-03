#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in float lifetimes_particle[];

uniform float lifetimeSeconds;

uniform vec3 cameraRight_world;
uniform vec3 cameraUp_world;
uniform mat4 viewProjectionMatrix;
uniform vec3 billboardPosition;

uniform vec2 initialScale;
uniform vec2 finalScale;

out vec2 fragmentUv;
out float lifetime_particle;

void main() {
	vec3 position = gl_in[0].gl_Position.xyz + billboardPosition.xyz;

	lifetime_particle = lifetimes_particle[0];
	float r = lifetime_particle / lifetimeSeconds;
	vec2 scale = mix(initialScale, finalScale, r);

	position -= cameraRight_world * scale.x * 0.5;
	position += cameraUp_world * scale.y * 0.5;
	gl_Position = viewProjectionMatrix * vec4(position, 1.0);
	fragmentUv = vec2(0.0, 0.0);
	EmitVertex();

	lifetime_particle = lifetimes_particle[0];
	position -= cameraUp_world * scale.y;
	gl_Position = viewProjectionMatrix * vec4(position, 1.0);
	fragmentUv = vec2(0.0, 1.0);
	EmitVertex();

	lifetime_particle = lifetimes_particle[0];
	position += cameraUp_world * scale.y;
	position += cameraRight_world * scale.x;
	gl_Position = viewProjectionMatrix * vec4(position, 1.0);
	fragmentUv = vec2(1.0, 0.0);
	EmitVertex();

	lifetime_particle = lifetimes_particle[0];
	position -= cameraUp_world * scale.y;
	gl_Position = viewProjectionMatrix * vec4(position, 1.0);
	fragmentUv = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}
