#version 430

out vec4 fragmentColor;
out vec4 glowColor;

void main() {
	fragmentColor = vec4(1.f, 0.5f, 0.f, 0.f);
	glowColor = fragmentColor;
}
