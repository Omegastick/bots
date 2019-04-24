#version 430 core

layout(location = 0)in vec2 vertex_position; 
layout(location = 1)in vec2 vertex_normal; 
layout(location = 2)in float vertex_miter; 
layout(location = 3)in vec4 vertex_color; 

out vec4 color; 

uniform mat4 u_mvp; 

void main()
 {
    vec2 position = vertex_position.xy + vec2(vertex_normal * vertex_miter);
    gl_Position = u_mvp * vec4(position, 0.0, 1.0);
    color = vertex_color;
}