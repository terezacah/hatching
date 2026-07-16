#version 450 core

//----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec3 position_ws;	  // The vertex position in world space.
	vec3 position_vs;	  // The vertex position in view space.
	vec3 normal_ws;		  // The vertex normal in world space.
	vec3 normal_vs;		  // The vertex normal in view space.
	vec2 tex_coord;		  // The vertex texture coordinates.
} in_data;

// The UBO with camera data.	
layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 projection;	  // The projection matrix.
	mat4 projection_inv;  // The inverse of the projection matrix.
	mat4 view;			  // The view matrix
	mat4 view_inv;		  // The inverse of the view matrix.
	mat3 view_it;		  // The inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;	  // The position of the eye in world space.
};

// The structure holding the information about a single Phong light.
struct PhongLight
{
	vec4 position;                   // The position of the light. Note that position.w should be one for point lights and spot lights, and zero for directional lights.
	vec3 ambient;                    // The ambient part of the color of the light.
	vec3 diffuse;                    // The diffuse part of the color of the light.
	vec3 specular;                   // The specular part of the color of the light. 
	vec3 spot_direction;             // The direction of the spot light, irrelevant for point lights and directional lights.
	float spot_exponent;             // The spot exponent of the spot light, irrelevant for point lights and directional lights.
	float spot_cos_cutoff;           // The cosine of the spot light's cutoff angle, -1 point lights, irrelevant for directional lights.
	float atten_constant;            // The constant attenuation of spot lights and point lights, irrelevant for directional lights. For no attenuation, set this to 1.
	float atten_linear;              // The linear attenuation of spot lights and point lights, irrelevant for directional lights.  For no attenuation, set this to 0.
	float atten_quadratic;           // The quadratic attenuation of spot lights and point lights, irrelevant for directional lights. For no attenuation, set this to 0.
};

// The UBO with light data.
layout (std140, binding = 2) uniform PhongLightsBuffer
{
	vec3 global_ambient_color;		 // The global ambient color.
	int lights_count;				 // The number of lights in the buffer.
	PhongLight lights[8];			 // The array with actual lights.
};

// The material data.
layout (std140, binding = 3) uniform PhongMaterialBuffer
{
    vec3 ambient;     // The ambient part of the material.
    vec3 diffuse;     // The diffuse part of the material.
    float alpha;      // The alpha (transparency) of the material.
    vec3 specular;    // The specular part of the material.
    float shininess;  // The shininess of the material.
} material;

// The flag determining whether a texture should be used.
uniform bool has_texture;
// The texture that will be used (if available).
layout(binding = 0) uniform sampler2D material_diffuse_texture;

// Shadow mapping
uniform bool use_shadows;
uniform mat4 shadow_matrix;
layout(binding = 1) uniform sampler2DShadow shadow_texture;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
layout(location = 0) out vec4 out_diffuse;
layout(location = 1) out vec4 out_position_vs;
layout(location = 2) out vec4 out_normal_vs;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
    vec3 N = normalize(in_data.normal_ws);

	vec3 dif = vec3(0.0);
	for (int i = 0; i < lights_count; i++)
    {
		// Compute light direction
		vec3 L_not_normalized = lights[i].position.xyz - in_data.position_ws * lights[i].position.w;
		vec3 L = normalize(L_not_normalized);

		// Lambert diffuse factor
		float ndotl = max(dot(N, L), 0.0);

		// Optional shadows
		float shadow_factor = 1.0;
		if (use_shadows)
        {
            vec4 shadow_coord = shadow_matrix * vec4(in_data.position_ws, 1.0);
            shadow_factor = textureProj(shadow_texture, shadow_coord);
        }

		dif += lights[i].diffuse * ndotl * shadow_factor;
	}

    //vec3 mat_diffuse = has_texture ? texture(material_diffuse_texture, in_data.tex_coord).rgb : material.diffuse;
	//dif *= mat_diffuse;

    out_diffuse = vec4(dif, 1.0);
	out_position_vs = vec4(in_data.position_vs, 1.0);
	out_normal_vs = vec4(normalize(in_data.normal_vs), 0.0);
}
