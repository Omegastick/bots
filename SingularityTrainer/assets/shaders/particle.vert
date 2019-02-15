#version 430 core

layout(location = 0)in vec2 v_position; 
layout(location = 1)in vec2 particle_position; 

out vec4 color; 

uniform mat4 u_mvp; 

void main()
 {
    gl_Position = u_mvp * vec4(v_position + particle_position, 0.0, 1.0);
    color = vec4(1.0);
}