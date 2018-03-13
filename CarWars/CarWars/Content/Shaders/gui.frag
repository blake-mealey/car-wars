#version 430

in vec2 fragmentUv;

uniform sampler2D diffuseTexture;
uniform vec4 diffuseColor;
uniform vec2 uvScale;

out vec4 fragmentColor;

void main() {
	fragmentColor = diffuseColor * texture(diffuseTexture, uvScale*fragmentUv);
}
