#version 430

in vec2 fragmentUv;

uniform vec2 uvScale;
uniform sampler2D diffuseTexture;

out vec4 color;
out vec4 glowColor;

void main () {
	color = texture(diffuseTexture, uvScale*vec2(1.f - fragmentUv.x, fragmentUv.y));
	glowColor = vec4(0.f);
}