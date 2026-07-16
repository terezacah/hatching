#version 450 core

//----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
    vec2 tex_coord;
} in_data;

// Texture with normals in view space
layout(binding = 0) uniform sampler2D normal_texture;
layout(binding = 1) uniform sampler2D depth_texture;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
layout(location = 0) out float edges;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
    // Texel size for sampling neighbors
    vec2 texel_size = 1.0 / textureSize(normal_texture, 0);

    // Normal at the current pixel
    vec3 normal = texture(normal_texture, in_data.tex_coord).xyz;

    // Depth at the current pixel
    float depth = texture(depth_texture, in_data.tex_coord).r;

    // Sobel kernel for horizontal edge detection
    float Gx[9] = float[9](
        +1, 0, -1,
        +2, 0, -2,
        +1, 0, -1
    );

    // Sobel kernel for vertical edge detection
    float Gy[9] = float[9](
        +1, +2, +1,
         0,  0,  0,
        -1, -2, -1
    );

    float sumX = 0.0;
    float sumY = 0.0;
    int k = 0;

    // Apply Sobel filter (loop over 3x3 neighborhood)
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            // Normal and depth of the neighbor
            vec3 normal_neigh = texture(normal_texture, in_data.tex_coord + vec2(x, y) * texel_size).xyz;
            float depth_neigh = texture(depth_texture, in_data.tex_coord + vec2(x, y) * texel_size).r;

            // Length of difference vector
            float normal_diff = length(normal - normal_neigh);

            // Depth difference
            float depth_diff = abs(depth - depth_neigh);

            // Combine normal and depth differences (depth tiny compared to normal length => scale it up)
            float combined = normal_diff + depth_diff * 20;

            // Accumulate weighted differences
            sumX += combined * Gx[k];
            sumY += combined * Gy[k];

            // Increment kernel index
            k++;
        }
    }

    // Gradient magnitude
    float g = sqrt(sumX * sumX + sumY * sumY);

    // Threshold to determine edges
    float threshold = 0.7;
    edges = g > threshold ? 1.0 : 0.0;
}
