#version 330

uniform sampler2D texture0;      // The main screen texture
uniform float scanline_count;    // Control the number of scanlines

in vec2 fragTexCoord;
out vec4 finalColor;

// Function to apply a curve effect to UV coordinates
vec2 uv_curve(vec2 uv) {
    uv = (uv - 0.5) * 2.0;

    // Curvature adjustments
    uv.x *= 1.0 + pow(abs(uv.y) / 6.0, 2.0);
    uv.y *= 1.0 + pow(abs(uv.x) / 6.0, 2.0);

    // Optional scaling effect
    //uv /= 1.2f;

    uv = (uv / 2.0) + 0.5;
    return uv;
}

void main() {
    float PI = 3.14159;

    // Apply curvature to the texture coordinates
    vec2 curvedUV = uv_curve(fragTexCoord);

    // Sample the main texture with slight color offsets for the CRT effect
    float r = texture(texture0, curvedUV + vec2(0.003, 0.0)).r;
    float g = texture(texture0, curvedUV + vec2(-0.003, 0.0)).g;
    float b = texture(texture0, curvedUV).b;

    // Optional scanline effect
    float s = sin(curvedUV.y * scanline_count * PI * 2.0);
    s = (s * 0.5 + 0.5) * 0.9 + 0.1;
    vec3 scan_line = vec3(pow(s, 0.25));

    // Combine RGB and scanline effects for final output
    finalColor = vec4(r, g, b, 1.0) * vec4(scan_line, 1.0);
}
