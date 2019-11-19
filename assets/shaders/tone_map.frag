#version 430 core

in vec2 tex_coord; 

out vec4 frag_color; 

uniform sampler2D u_texture; 

const float gamma = 0.1;

vec3 filmic_tone_mapping(vec3 color)
{
	color = max(vec3(0.), color - vec3(0.004));
	color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
}

void main()
{             
    vec3 hdr_color = texture(u_texture, tex_coord).rgb;
  
    vec3 mapped_color = filmic_tone_mapping(hdr_color);
  
    frag_color = vec4(mapped_color, 1.0);
}  