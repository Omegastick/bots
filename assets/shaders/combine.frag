#version 430 core

in vec2 tex_coord; 

out vec4 frag_color; 

uniform sampler2D u_texture_1; 
uniform sampler2D u_texture_2; 
uniform float u_amount_1;
uniform float u_amount_2;

void main()
{
    frag_color = (texture(u_texture_1, tex_coord) * u_amount_1) 
                  + (texture(u_texture_2, tex_coord) * u_amount_2);
}