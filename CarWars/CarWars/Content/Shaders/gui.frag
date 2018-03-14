#version 430

in vec2 fragmentUv;

uniform sampler2D diffuseTexture;
uniform vec4 diffuseColor;
uniform uint diffuseTextureEnabled;
uniform vec2 uvScale;

out vec4 fragmentColor;

void main() {
	fragmentColor = diffuseTextureEnabled * (diffuseColor * texture(diffuseTexture, uvScale*fragmentUv))
		+ (1 - diffuseTextureEnabled) * diffuseColor;
}
