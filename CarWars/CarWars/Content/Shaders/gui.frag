#version 430

in vec2 fragmentUv;

uniform sampler2D diffuseTexture;
uniform vec4 diffuseColor;
uniform uint diffuseTextureEnabled;
uniform vec2 uvScale;

uniform float materialEmissiveness;

out vec4 fragmentColor;
out vec4 glowColor;

void main() {
	fragmentColor = diffuseTextureEnabled * (diffuseColor * texture(diffuseTexture, uvScale*vec2(fragmentUv.x, 1 - fragmentUv.y)))
		+ (1 - diffuseTextureEnabled) * diffuseColor;
	glowColor = fragmentColor * materialEmissiveness;
}
