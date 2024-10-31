#version 330

uniform sampler2D texture0;          // Noise texture
uniform vec3 flameColor;             // Color of the flame
uniform float yOffset;               // Vertical offset
uniform float animationSpeed;        // Speed of the flame animation
uniform float time;                  // Time for animation

in vec2 fragTexCoord;
out vec4 finalColor;

void main() {
    // Offset UVs vertically based on time to animate the flame
    vec2 uv = fragTexCoord;
    uv.y += time * animationSpeed;

    // Sample the noise texture at the offset UVs
    vec4 noiseColor = texture(texture0, uv);

    // Manipulate color based on vertical position
    vec4 color = noiseColor;
    color.rgb += vec3(fragTexCoord.y - yOffset);
    color.rgb = step(color.rgb, vec3(0.5));
    color.rgb = vec3(1.0) - color.rgb;
    color.a = color.r;
    color.rgb *= flameColor;

    finalColor = color;
}
