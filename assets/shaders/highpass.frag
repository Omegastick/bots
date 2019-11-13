#version 430 core

in vec2 tex_coord; 

out vec4 frag_color; 

uniform sampler2D u_texture; 

void main()
{
    frag_color = texture(u_texture, tex_coord);
    float brightness = frag_color.r + frag_color.g + frag_color.b;
    if (brightness < 2.0) {
        frag_color = vec4(0);
    }
}