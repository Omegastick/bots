#version 430 core

in vec2 tex_coord; 

out vec4 frag_color; 

uniform vec2 u_offset;
uniform sampler2D u_texture; 

void main()
{
  vec4 color = vec4(0);
  color += 5.0 * texture(u_texture, tex_coord - u_offset);
  color += 6.0 * texture(u_texture, tex_coord);
  color += 5.0 * texture(u_texture, tex_coord + u_offset);
  frag_color = color / 16.0;
}