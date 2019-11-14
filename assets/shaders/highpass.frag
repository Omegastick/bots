#version 430 core

in vec2 tex_coord; 

out vec4 frag_color; 

uniform sampler2D u_texture; 

void main()
{
    frag_color = texture(u_texture, tex_coord);
    float brightness = dot(frag_color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness < 0.22) {
        frag_color = vec4(0);
    }
}