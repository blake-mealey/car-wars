#version 430

in vec2 fragmentUv;

uniform vec2 offset;
uniform sampler2D image;

out vec4 fragmentColor;

void main() {
	vec4 c = vec4(0.0);
	c += 5.0/16.0 * texture(image, fragmentUv - offset);
	c += 6.0/16.0 * texture(image, fragmentUv);
	c += 5.0/16.0 * texture(image, fragmentUv + offset);
	fragmentColor = c;
}
