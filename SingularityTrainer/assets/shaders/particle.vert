#version 430 core

layout(location = 0)in vec2 vertex_position; 
layout(location = 1)in vec2 particle_position; 
layout(location = 2)in vec2 particle_velocity; 
layout(location = 3)in float particle_start_time; 
layout(location = 4)in float particle_life_time; 
layout(location = 5)in float particle_size; 
layout(location = 6)in vec4 particle_start_color; 
layout(location = 7)in vec4 particle_end_color; 

out vec4 color; 

uniform mat4 u_mvp; 
uniform float u_time; 

void main()
 {
    float time_lived = u_time - particle_start_time;
    float remaining_life = max(particle_life_time - time_lived, 0);
    vec2 position = vertex_position * particle_size;
    position += particle_position;
    position += particle_velocity * time_lived;
    gl_Position = u_mvp * vec4(position, 0.0, 1.0);
    color = mix(particle_start_color, particle_end_color, time_lived / particle_life_time);
}