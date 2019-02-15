#version 430 core

layout(location = 0)in vec2 vertex_position; 
layout(location = 1)in vec2 particle_position; 
layout(location = 2)in vec2 particle_velocity; 
layout(location = 3)in float particle_start_time; 

out vec4 color; 

uniform mat4 u_mvp; 
uniform float u_time; 

void main()
 {
    vec2 position = vertex_position;
    position += particle_position;
    position += particle_velocity * (u_time - particle_start_time);
    gl_Position = u_mvp * vec4(position, 0.0, 1.0);
    color = vec4(1.0);
}