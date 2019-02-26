#version 430 core

in vec4 color; 
in vec2 tex_coord; 

out vec4 frag_color; 

uniform sampler2D u_texture; 

void main()
 {
    frag_color = texture(u_texture, tex_coord);
    frag_color *= color;
}