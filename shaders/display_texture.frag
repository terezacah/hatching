#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The transformation to be applied to the colors of the texture.
uniform mat4 transformation;
// The texture to display.
layout (binding = 0) uniform sampler2D input_tex;
// The flag determining if we are rendering a depth texture.
uniform bool show_depth;
// The flag determining if we are rendering in black and white.
uniform bool single_channel;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	if(show_depth){
		float original_depth = texture(input_tex, in_data.tex_coord).r;
		float normalized_depth = 2.0 * original_depth - 1.0;
		float near = 2.0;
		float far = 100.0;
		float linear_depth = ((2.0 * near * far) / (far + near - normalized_depth * (far - near))) / far;
		final_color = vec4(vec3(original_depth), 1.0);
	}else{
		if(single_channel){
			final_color = clamp(transformation * texture(input_tex, in_data.tex_coord).rrrr, vec4(0.0), vec4(1.0));
		}else{
			final_color = clamp(transformation * texture(input_tex, in_data.tex_coord), vec4(0.0), vec4(1.0));
		}
	}
}