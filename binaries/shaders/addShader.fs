#version 330

in vec2 tex_coords;

layout (location = 0) out vec4 out_color;

uniform sampler2D sampler;
uniform sampler2D copy;
uniform int sample;



void main()
{
    vec4 color = texture(sampler,tex_coords);
	vec4 asd = texture(copy, tex_coords);
    out_color = vec4((asd.xyz + (color.xyz * (1.0 / (sample + 1))))*(1.0-(1.0 / (sample + 2))),1.0);
}