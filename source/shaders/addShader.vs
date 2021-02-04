#version 150

in vec2 vertices;
out vec2 tex_coords;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 transformation;

void main()
{	
	tex_coords = vertices * 0.5 + 0.5;
	gl_Position = vec4(vertices,0.0,1.0);
}