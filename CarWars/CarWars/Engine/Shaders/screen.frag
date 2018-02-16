#version 430

in vec2 fragmentUv;

uniform sampler2D screen;

out vec4 fragmentColor;

void main() {
	fragmentColor = texture(screen, fragmentUv);
}
