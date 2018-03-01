#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 5) out;
in vec2 vFragmentUvs[];
out vec2 vFragmentUv;

void main() {
	vec4 position = gl_in[0].gl_Position;

	gl_Position = position + vec4(-0.1, 0.0, 0.0, 0.0);
	vFragmentUv = vFragmentUvs[0];
	EmitVertex();

	gl_Position = position + vec4(0.1, 0.0, 0.0, 0.0);
	vFragmentUv = vFragmentUvs[0];
	EmitVertex();

	gl_Position = position + vec4(0.0, 0.1, 0.0, 0.0);
	vFragmentUv = vFragmentUvs[0];
	EmitVertex();

	gl_Position = position + vec4(0.0, -0.1, 0.0, 0.0);
	vFragmentUv = vFragmentUvs[0];
	EmitVertex();

	gl_Position = position + vec4(-0.1, 0.0, 0.0, 0.0);
	vFragmentUv = vFragmentUvs[0];
	EmitVertex();

	EndPrimitive();
}