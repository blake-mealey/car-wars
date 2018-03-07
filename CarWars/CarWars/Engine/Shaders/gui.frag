#version 430

in vec2 fragmentUv;

uniform sampler2D diffuseTexture;
uniform vec2 uvScale;

out vec4 fragmentColor;

void main() {
	fragmentColor = texture(diffuseTexture, uvScale*vec2(1.f - fragmentUv.x, fragmentUv.y));
}
