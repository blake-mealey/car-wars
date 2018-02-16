#version 430

in vec3 fragmentUv;

uniform float sunSizeRadians;
uniform vec3 sunDirection;
uniform sampler2D sun;

uniform vec3 colorAdjust;
uniform samplerCube skybox;

uniform float time;

out vec4 fragmentColor;
out vec4 glowColor;

// From: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// From: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	// Get the adjusted skybox colour
	fragmentColor = texture(skybox, fragmentUv) * vec4(colorAdjust, 1);

	// Make the stars sparkle
	vec3 fragmentHsv = rgb2hsv(fragmentColor.xyz);
	float brightnessModifier = sin(time) * 0.3 * (fragmentHsv.z*fragmentHsv.z);
	fragmentHsv += vec3(0, 0, brightnessModifier + 2*abs(brightnessModifier));
	fragmentColor = vec4(hsv2rgb(fragmentHsv), fragmentColor.w);

	// Blend with sun colour
	float angle = acos(dot(normalize(-sunDirection), normalize(fragmentUv)));
	if (angle < sunSizeRadians - 0.01) {
		vec2 sunPos = vec2(angle/sunSizeRadians, 0.5);
		vec4 sunColor = texture(sun, sunPos);
		fragmentColor = (sunColor*(2 - pow(sunPos.x,1.5)) * sunColor.w) + (fragmentColor * (1 - sunColor.w));
	}

	vec3 hsv = rgb2hsv(fragmentColor.rgb);
	glowColor = vec4(fragmentColor.rgb * 0.25 * hsv.z * hsv.z, fragmentColor.a);
}
