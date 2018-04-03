#version 430

in vec2 fragmentUv;
in float lifetime_particle;

uniform vec2 uvScale;
uniform sampler2D diffuseTexture;

uniform vec4 initialColor;
uniform vec4 finalColor;
uniform float emissiveness;

uniform bool isSprite;
uniform vec2 spriteSize;
uniform vec2 textureSize;
uniform float animationCycles;

uniform float lifetimeSeconds;

out vec4 color;
out vec4 glowColor;

void main () {
	float r = lifetime_particle / lifetimeSeconds;

	vec2 uv = fragmentUv;
	if (isSprite) {
		vec2 spriteOffset = vec2(floor((r*animationCycles * (textureSize.x - spriteSize.x))/spriteSize.x) * spriteSize.x, 0);
		uv = (vec2(spriteOffset.x, textureSize.y - spriteOffset.y) + uv*vec2(spriteSize.x, -spriteSize.y)) / textureSize;
	}

	vec4 colorMult = mix(initialColor, finalColor, r);
	color = texture(diffuseTexture, uvScale*vec2(1.f - uv.x, uv.y)) * colorMult;
	glowColor = color * emissiveness;
}
