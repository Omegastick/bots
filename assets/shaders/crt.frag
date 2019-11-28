#version 120
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_texture;
uniform vec2 u_resolution;
const float output_gamma = 1.1;
const float strength = 0.5;

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution;
    
    vec3 color = texture2D(u_texture, uv).rgb;

    vec3 column_mask = mix(
        vec3(1.0, 0.8, 1.0),
        vec3(0.8, 1.0, 0.8),
        floor(mod(gl_FragCoord.x, 2.0))
    );

    vec3 row_mask = mix(
        vec3(1.0, 1.0, 1.0),
        vec3(0.2, 0.2, 0.2),
        floor(mod(gl_FragCoord.y, 2.0))
    );
    
    color *= column_mask;
    color *= row_mask;
    color = pow(color, vec3(1.0 / output_gamma));
    vec4 true_color = texture2D(u_texture, uv);
    
    gl_FragColor = vec4(mix(true_color.rgb, color, strength), 1.0);
}