#version 430 core

in vec2 position;
in vec3 inColor;
in vec2 inTexcoord;
out vec3 Color;
out vec2 texcoord;


void main() {
	Color = inColor;
	gl_Position = vec4(position, 0.0, 1.0);
	texcoord = inTexcoord;
}