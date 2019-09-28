#version 430 core

in vec2 tex_coord; 

out vec4 frag_color; 

uniform sampler2D u_texture; 
uniform sampler2D u_distortion;

void main()
{
    vec2 distorted_coord = tex_coord + texture(u_distortion, tex_coord).rg - 0.5;

    // frag_color = texture(u_texture, distorted_coord);
    frag_color = texture(u_distortion, tex_coord);
}