#version 430 core

layout(location = 0)in vec2 v_position; 
layout(location = 1)in vec2 v_tex_coord; 

out vec2 tex_coord; 

uniform mat4 u_mvp;

void main()
 {
    tex_coord = v_tex_coord;
    gl_Position = u_mvp * vec4(v_position, 0.0, 1.0);
}