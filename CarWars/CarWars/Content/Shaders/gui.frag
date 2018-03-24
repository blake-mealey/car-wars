#version 430

in vec2 fragmentUv;

uniform sampler2D diffuseTexture;
uniform vec4 diffuseColor;
uniform uint diffuseTextureEnabled;
uniform vec2 uvScale;

uniform float materialEmissiveness;

uniform bool isSprite;
uniform vec2 spriteSize;
uniform vec2 spriteOffset;
uniform vec2 textureSize;

out vec4 fragmentColor;
out vec4 glowColor;

void main() {
	vec2 uv = fragmentUv;
	if (isSprite) {
		uv = (vec2(spriteOffset.x, textureSize.y - spriteOffset.y) + uv*vec2(spriteSize.x, -spriteSize.y)) / textureSize;
	}

	fragmentColor = diffuseTextureEnabled * (diffuseColor * texture(diffuseTexture, uvScale*vec2(uv.x, 1 - uv.y)))
		+ (1 - diffuseTextureEnabled) * diffuseColor;
	glowColor = fragmentColor * materialEmissiveness;
}
