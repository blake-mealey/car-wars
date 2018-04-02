#version 430

in vec2 fragmentUv;
in float lifetime_particle;

uniform vec2 uvScale;
uniform sampler2D diffuseTexture;

uniform vec4 initialColor;
uniform vec4 finalColor;
uniform float emissiveness;

uniform float lifetimeSeconds;

out vec4 color;
out vec4 glowColor;

void main () {
	float r = lifetime_particle / lifetimeSeconds;
	vec4 colorMult = mix(initialColor, finalColor, r);
	color = texture(diffuseTexture, uvScale*vec2(1.f - fragmentUv.x, fragmentUv.y)) * colorMult;
	glowColor = color * emissiveness;
}