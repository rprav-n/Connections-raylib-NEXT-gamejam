#version 100
precision highp float;

uniform sampler2D texture0;          // Noise texture
uniform vec3 flameColor;             // Color of the flame
uniform float yOffset;               // Vertical offset
uniform float animationSpeed;        // Speed of the flame animation
uniform float time;                  // Time for animation

varying vec2 fragTexCoord;

void main() {
    // Offset UVs vertically based on time to animate the flame
    vec2 uv = fragTexCoord;
    uv.y = mod(uv.y + time * animationSpeed, 1.0);

    // Sample the noise texture at the offset UVs
    vec4 noiseColor = texture2D(texture0, uv);

    // Manipulate color based on vertical position
    vec4 color = noiseColor;
    color.rgb += vec3(fragTexCoord.y - yOffset);
    color.rgb = step(color.rgb, vec3(0.3));
    color.rgb = vec3(1.0) - color.rgb;
    color.a = color.r;
    color.rgb *= flameColor;

    gl_FragColor = color;
}