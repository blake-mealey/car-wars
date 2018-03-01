#version 430

in vec2 vFragmentUv;

uniform sampler2D diffuseTexture;

out vec4 fragmentColor;
out vec4 glowColor;

void main() {
	fragmentColor = texture(diffuseTexture, clamp(vFragmentUv, 0.01f, 0.99f));
	glowColor = fragmentColor * 0.25;
}
