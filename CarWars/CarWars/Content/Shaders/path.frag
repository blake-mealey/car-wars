#version 430

uniform vec3 diffuseColor;
uniform float materialEmissiveness;

out vec4 fragmentColor;
out vec4 glowColor;

void main() {
	fragmentColor = vec4(diffuseColor, 0.f);
	glowColor = fragmentColor * materialEmissiveness;
}
