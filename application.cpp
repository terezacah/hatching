// ################################################################################
// Common Framework for Computer Graphics Courses at FI MUNI.
//
// Copyright (c) Visitlab (https://visitlab.fi.muni.cz)
// All rights reserved.
// ################################################################################
#include "application.hpp"
#include "utils.hpp"

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments) : PV227Application(initial_width, initial_height, arguments)
{
	Application::compile_shaders();
	prepare_cameras();
	prepare_materials();
	prepare_textures();
	prepare_lights();
	prepare_scene();
	prepare_framebuffers();

	compute_random_samples();
	compute_random_tangent_directions();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
}

Application::~Application()
{
}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders()
{
	default_unlit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "unlit.frag");
	default_lit_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "lit.frag");
	display_texture_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "display_texture.frag");
	hatching_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "hatching.frag");
	diffuse_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "diffuse.frag");
	shadow_program = ShaderProgram(lecture_shaders_path / "object.vert", lecture_shaders_path / "nothing.frag");
	evaluate_ssao_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "evaluate_ssao.frag");
	blur_ssao_texture_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "blur_ssao_texture.frag");
	edges_program = ShaderProgram(lecture_shaders_path / "full_screen_quad.vert", lecture_shaders_path / "sobel.frag");

	std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras()
{
	// Sets the default camera position.
	camera.set_eye_position(glm::radians(-25.f), glm::radians(27.f), 20.f);
	// Computes the projection matrix.
	camera_ubo.set_projection(glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height), 2.0f, 100.0f));
	camera_ubo.update_opengl_data();

	// Sets the light camera data.
	light_camera_projection = glm::perspective(glm::radians(80.0f), 1.0f, 2.0f, 30.0f);
	light_camera_view = glm::mat4(1.0f);
	light_camera_data_ubo.set_projection(light_camera_projection);
	light_camera_data_ubo.set_view(light_camera_view);
	light_camera_data_ubo.update_opengl_data();
}

void Application::prepare_materials()
{
}

void Application::prepare_textures()
{
	wood_tex = TextureUtils::load_texture_2d(framework_textures_path / "wood.png");
	TextureUtils::set_texture_2d_parameters(wood_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);

	floor_tex = TextureUtils::load_texture_2d(framework_textures_path / "wall.png");
	TextureUtils::set_texture_2d_parameters(floor_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
}

void Application::prepare_lights()
{
	// The rest is set in the update scene method.
	phong_lights_ubo.set_global_ambient(glm::vec3(0.0f));
}

void Application::prepare_scene()
{
	// The cubes are animated so we update the model matrix later.
	cube1_object = SceneObject(cube, ModelUBO(), blue_material_ubo);
	scene_objects.push_back(cube1_object);

	// The cubes are animated so we update the model matrix later.
	cube2_object = SceneObject(cube, ModelUBO(), blue_material_ubo);
	scene_objects.push_back(cube2_object);

	// The torus is animated so we update the model matrix later.
	torus_object = SceneObject(torus, ModelUBO(), magenta_material_ubo);
	scene_objects.push_back(torus_object);

	const SceneObject sphere_object(sphere, ModelUBO(translate(glm::mat4(1.0f), glm::vec3(4.0f, 1.0f, 0.0f))), white_material_ubo, wood_tex);
	scene_objects.push_back(sphere_object);

	const SceneObject cylinder_object(cylinder, ModelUBO(translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -4.0f))), green_material_ubo);
	scene_objects.push_back(cylinder_object);

	const SceneObject teapot_object(teapot, ModelUBO(translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))), red_material_ubo);
	scene_objects.push_back(teapot_object);

	const SceneObject floor_object = SceneObject(cube, ModelUBO(translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f)) * scale(glm::mat4(1.0f), glm::vec3(10.0f, 0.1f, 10.0f))), white_material_ubo, floor_tex);
	scene_objects.push_back(floor_object);

	// The light model is placed based on the value from UI so we update its model matrix later.
	light_object = SceneObject(sphere, ModelUBO(), white_material_ubo);
}

void Application::prepare_framebuffers()
{
	// Create FBOs and set their draw buffers
	glCreateFramebuffers(1, &diffuse_fbo);
	glNamedFramebufferDrawBuffers(diffuse_fbo, 3, FBOUtils::draw_buffers_constants);

	glCreateFramebuffers(1, &shadow_fbo);
	glNamedFramebufferDrawBuffers(shadow_fbo, 1, FBOUtils::draw_buffers_constants);
	
	glCreateFramebuffers(1, &ssao_evaluation_fbo);
	glNamedFramebufferDrawBuffers(ssao_evaluation_fbo, 1, FBOUtils::draw_buffers_constants);

	glCreateFramebuffers(1, &ssao_blurring_fbo);
	glNamedFramebufferDrawBuffers(ssao_blurring_fbo, 1, FBOUtils::draw_buffers_constants);

	glCreateFramebuffers(1, &edges_fbo);
	glNamedFramebufferDrawBuffers(edges_fbo, 1, FBOUtils::draw_buffers_constants);

	resize_fullscreen_textures();
}

void Application::resize_fullscreen_textures()
{
	// Delete all existing textures
	glDeleteTextures(1, &diffuse_texture);
	glDeleteTextures(1, &depth_texture);
	glDeleteTextures(1, &shadow_texture);
	glDeleteTextures(1, &position_vs_texture);
	glDeleteTextures(1, &normal_vs_texture);
	glDeleteTextures(1, &ssao_occlusion_texture);
	glDeleteTextures(1, &ssao_blurred_occlusion_texture);
	glDeleteTextures(1, &edges_texture);

	// Create textures with new size, set their parameters and attach them to FBOs
    glCreateTextures(GL_TEXTURE_2D, 1, &diffuse_texture);
    glTextureStorage2D(diffuse_texture, 1, GL_RGBA8, width, height);
	TextureUtils::set_texture_2d_parameters(diffuse_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glNamedFramebufferTexture(diffuse_fbo, GL_COLOR_ATTACHMENT0, diffuse_texture, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &depth_texture);
    glTextureStorage2D(depth_texture, 1, GL_DEPTH_COMPONENT24, width, height);
	TextureUtils::set_texture_2d_parameters(depth_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glNamedFramebufferTexture(diffuse_fbo, GL_DEPTH_ATTACHMENT, depth_texture, 0);

	glCreateTextures(GL_TEXTURE_2D, 1, &shadow_texture);
	glTextureStorage2D(shadow_texture, 1, GL_DEPTH_COMPONENT24, shadow_tex_size, shadow_tex_size);
	TextureUtils::set_texture_2d_parameters(shadow_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glNamedFramebufferTexture(shadow_fbo, GL_DEPTH_ATTACHMENT, shadow_texture, 0);

	glCreateTextures(GL_TEXTURE_2D, 1, &position_vs_texture);
	glTextureStorage2D(position_vs_texture, 1, GL_RGBA32F, width, height);
	TextureUtils::set_texture_2d_parameters(position_vs_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glNamedFramebufferTexture(diffuse_fbo, GL_COLOR_ATTACHMENT1, position_vs_texture, 0);

	glCreateTextures(GL_TEXTURE_2D, 1, &normal_vs_texture);
	glTextureStorage2D(normal_vs_texture, 1, GL_RGBA16F, width, height);
	TextureUtils::set_texture_2d_parameters(normal_vs_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glNamedFramebufferTexture(diffuse_fbo, GL_COLOR_ATTACHMENT2, normal_vs_texture, 0);

	glCreateTextures(GL_TEXTURE_2D, 1, &ssao_occlusion_texture);
	glTextureStorage2D(ssao_occlusion_texture, 1, GL_R16F, width, height);
	TextureUtils::set_texture_2d_parameters(ssao_occlusion_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glNamedFramebufferTexture(ssao_evaluation_fbo, GL_COLOR_ATTACHMENT0, ssao_occlusion_texture, 0);

	glCreateTextures(GL_TEXTURE_2D, 1, &ssao_blurred_occlusion_texture);
	glTextureStorage2D(ssao_blurred_occlusion_texture, 1, GL_R16F, width, height);
	TextureUtils::set_texture_2d_parameters(ssao_blurred_occlusion_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glNamedFramebufferTexture(ssao_blurring_fbo, GL_COLOR_ATTACHMENT0, ssao_blurred_occlusion_texture, 0);

	glCreateTextures(GL_TEXTURE_2D, 1, &edges_texture);
	glTextureStorage2D(edges_texture, 1, GL_R8, width, height);
	TextureUtils::set_texture_2d_parameters(edges_texture, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	glNamedFramebufferTexture(edges_fbo, GL_COLOR_ATTACHMENT0, edges_texture, 0);

	// Check framebuffer status
	FBOUtils::check_framebuffer_status(diffuse_fbo, "DiffuseFBO");
	FBOUtils::check_framebuffer_status(shadow_fbo, "ShadowFBO");
	FBOUtils::check_framebuffer_status(ssao_evaluation_fbo, "Evaluate SSAO");
	FBOUtils::check_framebuffer_status(ssao_blurring_fbo, "Blurring SSAO");
	FBOUtils::check_framebuffer_status(edges_fbo, "Outlines FBO");
}

void Application::compute_random_samples()
{
	const int number_of_ssao_samples = 64;

	std::vector<glm::vec4> ssao_samples(number_of_ssao_samples);
	for (size_t i = 0; i < ssao_samples.size(); i++)
	{
		// Create a uniform point on a hemisphere (unit sphere, cut by xy plane, hemisphere in +z direction)
		const float alpha = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * static_cast<float>(M_PI);
		const float beta = acos(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
		glm::vec3 point_on_hemisphere = glm::vec3(sinf(alpha) * cosf(beta), sinf(alpha) * sinf(beta), cosf(beta));

		// Place the point on the hemisphere inside the hemisphere, so that the points in the hemisphere were uniformly
		// distributed in the hemisphere. This is true when the distance of the points from the center of the hemisphere
		// is the cube root of a uniformly distributed random number from interval [0,1].
		const float radius = powf(static_cast<float>(rand()) / static_cast<float>(RAND_MAX), 1.0f / 3.0f);
		glm::vec3 point_in_hemisphere = point_on_hemisphere * radius;

		ssao_samples[i] = glm::vec4(point_in_hemisphere, 0.0f);
	}

	// Creates a buffer for storing the random positions.
	glCreateBuffers(1, &ssao_samples_ubo);
	glNamedBufferStorage(ssao_samples_ubo, sizeof(float) * 4 * ssao_samples.size(), ssao_samples.data(), 0 /* no updates needed*/);
}

void Application::compute_random_tangent_directions()
{
	std::vector<glm::vec3> SSAORandomTangent(16);
	for (int i = 0; i < 16; i++)
	{
		// Generates the tangents as random directions in xy plane.
		const float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * static_cast<float>(M_PI);
		SSAORandomTangent[i] = glm::vec3(sinf(angle), cosf(angle), 0.0f);
	}
	// Creates a 4x4 texture for these random directions.
	glCreateTextures(GL_TEXTURE_2D, 1, &ssao_random_tangent_vs_texture);
	glTextureStorage2D(ssao_random_tangent_vs_texture, 1, GL_RGBA32F, 4, 4);
	glTextureSubImage2D(ssao_random_tangent_vs_texture, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, SSAORandomTangent.data());
	TextureUtils::set_texture_2d_parameters(ssao_random_tangent_vs_texture, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void Application::update(float delta)
{
	PV227Application::update(delta);

	// Updates the main camera.
	const glm::vec3 eye_position = camera.get_eye_position();
	camera_ubo.set_view(lookAt(eye_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	camera_ubo.update_opengl_data();

	// Computes the light position.
	const glm::vec3 light_position = glm::vec3(15, 15, 15) * glm::vec3(cosf(gui_light_position / 6.0f) * sinf(gui_light_position), sinf(gui_light_position / 6.0f), cosf(gui_light_position / 6.0f) * cosf(gui_light_position));

	// Updates the light model visible in the scene.
	light_object.get_model_ubo().set_matrix(translate(glm::mat4(1.0f), light_position) * scale(glm::mat4(1.0f), glm::vec3(0.2f)));
	light_object.get_model_ubo().update_opengl_data();

	// Updates the OpenGL buffer storing the information about the light.
	phong_lights_ubo.clear();
	phong_lights_ubo.add(PhongLightData::CreateSpotLight(light_position, glm::vec3(0.05f), glm::vec3(0.95f), glm::vec3(1.0f), -normalize(light_position), 20.0f, cosf(glm::radians(40.0f))));
	phong_lights_ubo.update_opengl_data();

	// Update the view matrix for the light camera using glm::lookAt function.
	light_camera_view = glm::lookAt(glm::vec3(phong_lights_ubo.get_lights()[0].position), glm::vec3(phong_lights_ubo.get_lights()[0].position) + phong_lights_ubo.get_lights()[0].spot_direction, glm::vec3(0.0f, 1.0f, 0.0f));
	light_camera_data_ubo.set_view(light_camera_view);
	light_camera_data_ubo.update_opengl_data();

	// Animates the objects.
	torus_object.get_model_ubo().set_matrix(
		translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 4.0f)) * rotate(glm::mat4(1.0f), static_cast<float>(elapsed_time) * 0.002f, glm::vec3(0.0f, 1.0f, 0.0f)) * rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	torus_object.get_model_ubo().update_opengl_data();

	cube1_object.get_model_ubo().set_matrix(translate(glm::mat4(1.0f), glm::vec3(-4.0f, 1.0f, 4.0f * sin(elapsed_time * 0.0005))));
	cube1_object.get_model_ubo().update_opengl_data();
	cube2_object.get_model_ubo().set_matrix(translate(glm::mat4(1.0f), glm::vec3(-4.0f, 1.0f, -4.0f * sin(elapsed_time * 0.0005))));
	cube2_object.get_model_ubo().update_opengl_data();

	// Computes the shadow matrix.
	shadow_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)) * light_camera_projection * light_camera_view;
}

// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render()
{
	// Starts measuring the elapsed time.
	glBeginQuery(GL_TIME_ELAPSED, render_time_query);

	if (what_to_display == DISPLAY_DEFAULT_SCENE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
		render_scene(default_unlit_program);
	}
	else if (what_to_display == DISPLAY_SHADOW_MAP)
	{
		render_shadow_map();
		show_textures();
	}
	else if (what_to_display == DISPLAY_DIFFUSE_SCENE || what_to_display == DISPLAY_POSITIONS_VS || 
		what_to_display == DISPLAY_NORMALS_VS || what_to_display == DISPLAY_DEPTH)
	{
		prepare_for_diffuse_pass();
		render_scene(diffuse_program, false);
		show_textures();
	}
	else if (what_to_display == DISPLAY_DIFFUSE_SHADOWS)
	{
		render_shadow_map();
		prepare_for_diffuse_pass();
		render_scene(diffuse_program, true);
		show_textures();
	}
	else if (what_to_display == AMBIENT_OCCLUSION)
	{
		prepare_for_diffuse_pass();
		render_scene(diffuse_program, false);
		evaluate_ssao();
		apply_blur_ssao();
		show_textures();
	}
	else if (what_to_display == DISPLAY_EDGES)
	{
		prepare_for_diffuse_pass();
		render_scene(diffuse_program, false);
		evaluate_edges();
		show_textures();
	}
	else if (what_to_display == DISPLAY_EXPANDED_EDGES)
	{
		// Not implemented
		show_textures();
	}
	else if (what_to_display == DISPLAY_FINAL_IMAGE)
	{
		render_shadow_map();
		prepare_for_diffuse_pass();
		render_scene(diffuse_program, true);
		if (use_ao)
		{
			evaluate_ssao();
			apply_blur_ssao();
		} 
		else
		{
			clear_ssao_buffers();
		}
		evaluate_edges();
		show_textures();
	}	

	// Resets the VAO and the program.
	glBindVertexArray(0);
	glUseProgram(0);

	// Stops measuring the elapsed time.
	glEndQuery(GL_TIME_ELAPSED);

	// Waits for OpenGL - don't forget OpenGL is asynchronous.
	glFinish();

	// Evaluates the query.
	GLuint64 render_time;
	glGetQueryObjectui64v(render_time_query, GL_QUERY_RESULT, &render_time);
	fps_gpu = 1000.f / (static_cast<float>(render_time) * 1e-6f);
}

void Application::prepare_for_diffuse_pass() {
	glBindFramebuffer(GL_FRAMEBUFFER, diffuse_fbo);
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
}

void Application::render_scene(ShaderProgram& program, bool use_shadows)
{
	//camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
	phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);
	program.use();

	if (use_shadows)
    {
        program.uniform_matrix("shadow_matrix", shadow_matrix);
        program.uniform("use_shadows", true);

        glBindTextureUnit(1, shadow_texture);
        glTextureParameteri(shadow_texture, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTextureParameteri(shadow_texture, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    else
    {
        program.uniform("use_shadows", false);
    }

	for (SceneObject& object : scene_objects)
	{
		if (object.has_texture())
		{
			program.uniform("has_texture", true);
			glBindTextureUnit(0, object.get_texture());
		}
		else
		{
			program.uniform("has_texture", false);
		}
		object.get_material().bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
		object.get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
		object.get_geometry().bind_vao();
		object.get_geometry().draw();
	}

	light_object.get_material().bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
	light_object.get_model_ubo().bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
	light_object.get_geometry().bind_vao();
	light_object.get_geometry().draw();
}

void Application::show_textures() const {
    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    // We do not need depth test.
    glDisable(GL_DEPTH_TEST);

    // Selects the program and binds the appropriate textures.
    if (what_to_display == DISPLAY_FINAL_IMAGE) {
        hatching_program.use();

        glBindTextureUnit(0, diffuse_texture);
    	glBindTextureUnit(1, depth_texture);
		glBindTextureUnit(2, ssao_blurred_occlusion_texture);
    	glBindTextureUnit(3, edges_texture);
		
		hatching_program.uniform("use_ao", use_ao);
        hatching_program.uniform("resolution", glm::vec2(width, height));
    } else {
        display_texture_program.use();
        display_texture_program.uniform("show_depth", false);
        display_texture_program.uniform("single_channel", false);
		display_texture_program.uniform_matrix("transformation", glm::mat4(1.0f));

        if (what_to_display == DISPLAY_DIFFUSE_SCENE) 
		{
            glBindTextureUnit(0, diffuse_texture);
        }
		else if (what_to_display == DISPLAY_SHADOW_MAP)
		{
			display_texture_program.uniform("show_depth", true);
			glBindTextureUnit(0, shadow_texture);
		}
		else if (what_to_display == DISPLAY_DIFFUSE_SHADOWS)
		{
			glBindTextureUnit(0, diffuse_texture);
		}
		else if (what_to_display == DISPLAY_DEPTH)
		{
			display_texture_program.uniform("show_depth", true);
			display_texture_program.uniform("single_channel", true);
			glBindTextureUnit(0, depth_texture);
		}
		else if (what_to_display == DISPLAY_POSITIONS_VS)
		{
			display_texture_program.uniform_matrix("transformation", scale(glm::mat4(1.0f), glm::vec3(0.15f)));
			glBindTextureUnit(0, position_vs_texture);
		}
		else if (what_to_display == DISPLAY_NORMALS_VS)
		{
			glBindTextureUnit(0, normal_vs_texture);
		}
		else if (what_to_display == AMBIENT_OCCLUSION)
		{
			display_texture_program.uniform("single_channel", true);
			glBindTextureUnit(0, ssao_blurred_occlusion_texture);
		}
		else if (what_to_display == DISPLAY_EDGES)
		{
			display_texture_program.uniform("single_channel", true);
			glBindTextureUnit(0, edges_texture);
		}
		else if (what_to_display == DISPLAY_EXPANDED_EDGES)
		{
			glBindTextureUnit(0, wood_tex);
		}
    }

    // Renders the full screen quad to evaluate every pixel.
	// Binds an empty VAO as we do not need any state.
	glBindVertexArray(empty_vao);
	// Calls a draw command with 3 vertices that are generated in vertex shader.
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::render_shadow_map() 
{
	// Bind the shadow map FBO, set viewport
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
	glViewport(0, 0, shadow_tex_size, shadow_tex_size);

	// Clear depth buffer, enable depth test
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Front face culling to fix shadow acne
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// Bind the light camera UBO
	light_camera_data_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);

	// Render the scene from the light's point of view
	render_scene(shadow_program, false);

	glDisable(GL_CULL_FACE);
}

void Application::evaluate_ssao() const
{
    // 1. Bind SSAO evaluation FBO
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_evaluation_fbo);
    glViewport(0, 0, width, height);

    // 2. Bind the G-buffer textures created in your diffuse pass
    glBindTextureUnit(0, position_vs_texture);              // view-space positions
    glBindTextureUnit(1, normal_vs_texture);                // view-space normals
    glBindTextureUnit(2, ssao_random_tangent_vs_texture); // noise texture (4x4)

    // 3. Bind camera + SSAO samples UBOs
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ssao_samples_ubo);

    // 4. Activate SSAO shader
    evaluate_ssao_program.use();
    evaluate_ssao_program.uniform("ssao_radius", ssao_radius);

    // 5. Render full-screen triangle
    glBindVertexArray(empty_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::apply_blur_ssao() const
{
    // 1. Bind blur FBO
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_blurring_fbo);
    glViewport(0, 0, width, height);

    // 2. Bind SSAO raw texture
    glBindTextureUnit(0, ssao_occlusion_texture);

    // 3. Use blur shader
    blur_ssao_texture_program.use();

    // 4. Render full-screen triangle
    glBindVertexArray(empty_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::clear_ssao_buffers() const
{
	// Clears the evaluation framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, ssao_evaluation_fbo);
	glViewport(0, 0, width, height);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Clears the blurring framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, ssao_blurring_fbo);
	glViewport(0, 0, width, height);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Application::evaluate_edges()
{
	// Bind edges FBO, set viewport, clear color buffer
    glBindFramebuffer(GL_FRAMEBUFFER, edges_fbo);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind normal and depth textures for detecting outlines
    glBindTextureUnit(0, normal_vs_texture);
	glBindTextureUnit(1, depth_texture);

	// Use Sobel shader
	edges_program.use();

	// Render full-screen triangle
    glBindVertexArray(empty_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui()
{
	const float unit = ImGui::GetFontSize();

	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
	ImGui::SetWindowSize(ImVec2(27 * unit, 12 * unit));
	ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

	std::string fps_cpu_string = "FPS (CPU): ";
	ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

	std::string fps_string = "FPS (GPU): ";
	ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

	ImGui::Combo("Display", &what_to_display, DISPLAY_LABELS, IM_ARRAYSIZE(DISPLAY_LABELS));

	ImGui::Checkbox("Ambient Occlusion", &use_ao);

	ImGui::SliderInt("Edge Thickness", &outline_thickness, 1, 10);

	ImGui::SliderAngle("Light Position", &gui_light_position, 0);
	ImGui::End();
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------
void Application::on_resize(int width, int height)
{
	PV227Application::on_resize(width, height);
	resize_fullscreen_textures();
}
