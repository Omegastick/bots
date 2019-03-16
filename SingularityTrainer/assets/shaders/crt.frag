#version 120
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform vec2 u_virtual_resolution;
uniform float u_output_gamma;
uniform float u_strength;
uniform float u_distortion_factor;

vec2 distort(vec2 xy, float u_distortion_factor)
 {
    vec2 cc = xy - 0.5;
    float distortion = dot(cc, cc) * u_distortion_factor;
    return (xy + cc * (1.0 + distortion) * distortion);
}

vec4 scanline_weights(float distance_from_scanline, vec4 color)
 {
    vec4 width = 2.0 + 2.0 * pow(color, vec4(4.0));
    vec4 weights = vec4(distance_from_scanline / 0.3);
    return 1.4 * exp(-pow(weights * inversesqrt(0.5 * width), width)) / (0.6 + 0.2 * width);
}

void main() {
    // vec2 xy = distort(gl_FragCoord.xy / u_resolution.xy, u_distortion_factor);
    vec2 xy = gl_FragCoord.xy / u_resolution;
    // vec2 xy_inverted = distort(gl_FragCoord.xy / u_resolution.xy, - u_distortion_factor);
    vec2 xy_inverted = gl_FragCoord.xy / u_resolution;
    
    vec2 xy_scaled = xy * u_virtual_resolution;
    vec2 xy_fract = fract(xy_scaled * 10);
    
    vec4 color = texture2D(u_texture, xy);
    vec4 color_2 = texture2D(u_texture, xy + vec2(0.0, 1.0 / u_virtual_resolution.y));
    
    vec4 weights = scanline_weights(xy_fract.y, color);
    vec4 weights_2 = scanline_weights(1.0 - xy_fract.y, color_2);
    vec3 distorted_color = (color * weights + color_2 * weights_2).rgb;
    
    vec3 dot_mask = mix(
        vec3(1.0, 0.7, 1.0),
        vec3(0.7, 1.0, 0.7),
        floor(mod(xy_scaled.x, 2.0))
    );
    
    distorted_color *= dot_mask;
    distorted_color = pow(distorted_color, vec3(1.0 / u_output_gamma));
    vec4 true_color = texture2D(u_texture, xy);
    
    gl_FragColor = vec4(mix(true_color.rgb, distorted_color, u_strength), 1.0);
}