precision mediump float;

uniform sampler2D texture;
uniform vec2 resolution;

#define distortion_factor 0.03;

vec2 disort(vec2 coord)
{
		vec2 cc = coord - 0.5;
		float distortion = dot(cc, cc) * distortion_factor;
		return (coord + cc * (1.0 + distortion) * distortion);
}

vec4 scanline_weights(float distance_from_scanline, vec4 color)
{
        vec4 width = 2.0 + 2.0 * pow(color, vec4(4.0));

        vec4 weights = vec4(distance_from_scanline / 0.3);
        return 1.4 * exp(-pow(weights * inversesqrt(0.5 * width), width)) / (0.6 + 0.2 * width);
}


void main() {
    vec2 coord = disort(gl_FragCoord.xy/resolution.xy);

    vec4 color  = texture2D(texture, coord);
    vec4 color_2 = texture2D(texture, coord);

    float distance_from_scanline = mod(coord.y, 1. / resolution.y);
    vec4 weights = scanline_weights(distance_from_scanline, color);
    vec4 weights_2 = scanline_weights(1. - distance_from_scanline, color_2);
    vec3 distorted_color  = (color * weights + color_2 * weights_2).rgb;

    vec3 dot_mask = mix(
        vec3(1.0, 0.9, 1.0),
        vec3(0.9, 1.0, 0.9),
        floor(mod(coord.y * resolution.y, 2.0))
    );
    
    distorted_color *= dot_mask;

    gl_FragColor = vec4(distorted_color, 1.0);
}