#version 430 core

in vec4 color; 
in vec2 tex_coord; 

out vec4 frag_color; 

uniform sampler2D u_texture; 
uniform vec2 u_resolution; 
uniform vec2 u_direction; 

void main()
 {
    frag_color = vec4(0.0);
    vec2 offset_1 = vec2(1.411764705882353) * u_direction;
    vec2 offset_2 = vec2(3.2941176470588234) * u_direction;
    vec2 offset_3 = vec2(5.176470588235294) * u_direction;
    frag_color += texture2D(u_texture, tex_coord) * 0.1964825501511404;
    frag_color += texture2D(u_texture, tex_coord + (offset_1 / u_resolution)) * 0.2969069646728344;
    frag_color += texture2D(u_texture, tex_coord - (offset_1 / u_resolution)) * 0.2969069646728344;
    frag_color += texture2D(u_texture, tex_coord + (offset_2 / u_resolution)) * 0.09447039785044732;
    frag_color += texture2D(u_texture, tex_coord - (offset_2 / u_resolution)) * 0.09447039785044732;
    frag_color += texture2D(u_texture, tex_coord + (offset_3 / u_resolution)) * 0.010381362401148057;
    frag_color += texture2D(u_texture, tex_coord - (offset_3 / u_resolution)) * 0.010381362401148057;
    frag_color *= color;
}