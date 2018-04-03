#version 430

in vec2 fragmentUv;

uniform sampler2D screen;

out vec4 fragmentColor;

void main() {
	fragmentColor = vec4(texture(screen, fragmentUv).rgb, 1);
}
