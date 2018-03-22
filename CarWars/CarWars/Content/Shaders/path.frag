#version 430

uniform vec4 diffuseColor;
uniform float materialEmissiveness;

out vec4 fragmentColor;
out vec4 glowColor;

void main() {
	fragmentColor = diffuseColor;
	glowColor = fragmentColor * materialEmissiveness;
}
