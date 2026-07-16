#version 450 core

//----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
    vec2 tex_coord;
} in_data;

// Uniforms
uniform vec2 resolution;
uniform bool use_ao;  

// Textures
layout(binding = 0) uniform sampler2D diffuse_texture;
layout(binding = 1) uniform sampler2D depth_texture;
layout(binding = 2) uniform sampler2D ao_texture;
layout(binding = 3) uniform sampler2D edge_texture;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
layout(location = 0) out vec4 fragColor;

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------
float rand(float x) 
{
    return fract(sin(x) * 43758.5453);
}

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
    // UV coordinates
    vec2 uv_tex = in_data.tex_coord;

    // Read diffuse value
    float diffuse = texture(diffuse_texture, uv_tex).r;

    // Read depth value
    float depth = texture(depth_texture, uv_tex).r;

    // Read ambient occlusion value
    float ao = 1.0;
    if (use_ao) {
        ao = texture(ao_texture, uv_tex).r;
    }

    // Read edge value (0 = no edge, 1 = edge)
    float edges = texture(edge_texture, uv_tex).r;

    // Compute hatching amount
    float amount = diffuse * ao * (1.0 - edges); // amount: 1 = no hatching, 0 = max hatching 

    // Just a small amount of hatching for background pixels
    float alpha = texture(diffuse_texture, uv_tex).a;
    if (alpha == 0.0) { //  && edges < 0.5
        amount = 0.9;
    }

    // Normalize uv coordinates to [0,1]
    vec2 uv = gl_FragCoord.xy / resolution;

    // Scale uv coordinates
    float ratio = resolution.x / resolution.y;
    vec2 p = 6.0 * (uv * ratio);

    // Wave-like distortion
    p += vec2(sin(4.0 * p.y) * 0.03, sin(4.0 * p.x) * 0.03);

    // Add some noise
    p += 0.01 * vec2(rand(p.x * 3.1 + p.y * 8.7), rand(p.x * 1.1 + p.y * 6.7));

    // Compute hatching patterns
    float hx = sin(p.x * 190.0 + p.y * 130.0) * 0.5 + 0.5 - amount;
    float hy = sin(p.x * -130.0 + p.y * 190.0) * 0.5 + 0.5 - amount - 0.4;
    float hatching = max(clamp(hx, 0.0, 1.0), clamp(hy, 0.0, 1.0));

    // Set color
    vec4 lightColor = vec4(1.0, 0.9, 0.8, 1.0);
    vec4 darkColor  = lightColor * 0.2;

    fragColor = mix(lightColor, darkColor, hatching);;
}
