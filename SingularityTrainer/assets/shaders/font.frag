#version 430 core

in vec4 color; 
in vec2 tex_coord; 

out vec4 frag_color; 

uniform sampler2D u_texture; 

void main()
 {
    frag_color = vec4(texture(u_texture, tex_coord).r);
    frag_color *= color;
}