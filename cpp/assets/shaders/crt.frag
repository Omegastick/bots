#version 120
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texture;
uniform vec2 screen_resolution;
uniform vec2 texture_resolution;
uniform float output_gamma;
uniform float strength;

#define distortion_factor 0.08;

vec2 distort(vec2 xy)
{
	vec2 cc = xy - 0.5;
	float distortion = dot(cc, cc) * distortion_factor;
	return (xy + cc * (1.0 + distortion) * distortion);
}

vec4 scanline_weights(float distance_from_scanline, vec4 color)
{
    vec4 width = 2.0 + 2.0 * pow(color, vec4(4.0));
    vec4 weights = vec4(distance_from_scanline / 0.3);
    return 1.4 * exp(-pow(weights * inversesqrt(0.5 * width), width)) / (0.6 + 0.2 * width);
}

void main() {
    float aspect_ratio = screen_resolution.x / screen_resolution.y;
    vec2 xy = distort(gl_FragCoord.xy/screen_resolution.xy);

    vec2 ratio_scale = xy * texture_resolution.xy - vec2(0.5);
    vec2 uv_ratio = fract(ratio_scale);

    vec4 color = texture2D(texture, xy);
    vec4 color_2 = texture2D(texture, xy + vec2(0.0, 1. / texture_resolution.y));

    vec4 weights = scanline_weights(uv_ratio.y, color);
    vec4 weights_2 = scanline_weights(1. - uv_ratio.y, color_2);
    vec3 distorted_color = (color * weights + color_2 * weights_2).rgb;

    vec3 dot_mask = mix(
        vec3(1.0, 0.9, 1.0),
        vec3(0.9, 1.0, 0.9),
        floor(mod(xy.x * texture_resolution.x * aspect_ratio, 2.0))
    );
    
    distorted_color *= dot_mask;
    distorted_color = pow(distorted_color, vec3(1.0 / output_gamma));
    vec4 true_color = texture2D(texture, xy);

    gl_FragColor = vec4(mix(true_color.rgb, distorted_color, strength), 1.0);
}