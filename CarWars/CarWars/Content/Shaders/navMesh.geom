#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
in vec2 vFragmentUvs[];
out vec2 vFragmentUv;

void main() {
	vec4 position = gl_in[0].gl_Position;

	float scale = 0.5f;

	gl_Position = position + vec4(-scale, 0.0, 0.0, 0.0);
	vFragmentUv = vFragmentUvs[0];
	EmitVertex();

	gl_Position = position + vec4(0.0, scale, 0.0, 0.0);
	vFragmentUv = vFragmentUvs[0];
	EmitVertex();

	gl_Position = position + vec4(0.0, -scale, 0.0, 0.0);
	vFragmentUv = vFragmentUvs[0];
	EmitVertex();

	gl_Position = position + vec4(scale, 0.0, 0.0, 0.0);
	vFragmentUv = vFragmentUvs[0];
	EmitVertex();

	EndPrimitive();
}
