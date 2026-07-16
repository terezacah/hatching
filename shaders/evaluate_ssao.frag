#version 450 core

// The number of the samples.
#define NUMBER_OF_SSAO_SAMPLES 64

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;  // The vertex texture coordinates.
} in_data;

// The UBO with camera data.	
layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 projection;		// The projection matrix.
	mat4 projection_inv;	// The inverse of the projection matrix.
	mat4 view;				// The view matrix
	mat4 view_inv;			// The inverse of the view matrix.
	mat3 view_it;			// The inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;		// The position of the eye in world space.
};

// The random positions of the SSAO kernel.
layout (std140, binding = 1) uniform KernelSamples
{
	vec4 kernel_samples[NUMBER_OF_SSAO_SAMPLES];
};

// The texture with positoins in view space.
layout (binding = 0) uniform sampler2D positions_vs_tex;
// The texture with normals in view space.
layout (binding = 1) uniform sampler2D normals_vs_tex;
// The texture with random tangents in view space.
layout (binding = 2) uniform sampler2D random_tangent_vs_tex;

// The radius of the SSAO hemisphere which is sampled.
uniform float ssao_radius;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The computed occlusion.
layout (location = 0) out float final_occlusion;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	vec4 position_vs = texture(positions_vs_tex, in_data.tex_coord);
	vec3 normal_vs = normalize(texture(normals_vs_tex, in_data.tex_coord).xyz);

	vec3 tangent_vs = texture(random_tangent_vs_tex, gl_FragCoord.xy / vec2(textureSize(random_tangent_vs_tex, 0))).xyz;
	vec3 bitangent_vs = normalize(cross(normal_vs, tangent_vs));
	tangent_vs = normalize(cross(bitangent_vs, normal_vs));
	mat3 TBN = mat3(tangent_vs, bitangent_vs, normal_vs);

	int occluded_samples = 0, not_occluded_samples = 0;
	for (int i = 0; i < NUMBER_OF_SSAO_SAMPLES; i++)
	{
		vec3 sample_offset_vs = TBN * kernel_samples[i].xyz * ssao_radius;
		vec3 sample_position_vs = position_vs.xyz + sample_offset_vs;
		vec4 sample_position_cs = projection * vec4(sample_position_vs, 1.0);
		vec3 sample_position_nds = sample_position_cs.xyz / sample_position_cs.w;
		vec4 closest_object_vs = textureLod(positions_vs_tex, sample_position_nds.xy * 0.5 + 0.5, 0);

		if (closest_object_vs.w == 0.0)
		{
			not_occluded_samples++;
		}
		else
		{
			float sample_distance = -sample_position_vs.z;	
			float occluder_distance = -closest_object_vs.z;  
			if ((sample_distance - occluder_distance) >= ssao_radius)
			{
			}
			else 
			{
				if (sample_distance > occluder_distance)
					occluded_samples++;
				else
					not_occluded_samples++;
			}
		}
	}

	if (position_vs.w == 0.0)
		final_occlusion = 0.0;		
	else if ((occluded_samples + not_occluded_samples) == 0)
		final_occlusion = 1.0;		
	else
		final_occlusion = float(not_occluded_samples) / float(occluded_samples + not_occluded_samples);
}