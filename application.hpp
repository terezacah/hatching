#pragma once
#include "camera_ubo.hpp"
#include "light_ubo.hpp"
#include "pv227_application.hpp"
#include "scene_object.hpp"

class Application : public PV227Application
{
	// ----------------------------------------------------------------------------
	// Variables (Geometry)
	// ----------------------------------------------------------------------------
protected:
	/** The scene objects. */
	std::vector<SceneObject> scene_objects;

	/** The scene object representing the light. */
	SceneObject light_object;
	/** The scene object representing the first cube. */
	SceneObject cube1_object;
	/** The scene object representing the second cube. */
	SceneObject cube2_object;
	/** The scene object representing the torus. */
	SceneObject torus_object;

	// ----------------------------------------------------------------------------
	// Variables (Textures)
	// ----------------------------------------------------------------------------
protected:
	/** The wood texture. */
	GLuint wood_tex;
	/** The floor texture. */
	GLuint floor_tex;

	// ----------------------------------------------------------------------------
	// Variables (Light)
	// ----------------------------------------------------------------------------
protected:
	/** The UBO storing the data about lights - positions, colors, etc. */
	PhongLightsUBO phong_lights_ubo;

	// ----------------------------------------------------------------------------
	// Variables (Camera)
	// ----------------------------------------------------------------------------
protected:
	/** The UBO storing the information about camera. */
	CameraUBO camera_ubo;

	CameraUBO light_camera_data_ubo;
	glm::mat4 light_camera_projection;
	glm::mat4 light_camera_view;

	// ----------------------------------------------------------------------------
	// Variables (Shaders)
	// ----------------------------------------------------------------------------
protected:
	/** The shader program used to display the textures. */
	ShaderProgram display_texture_program;

	ShaderProgram hatching_program;
	ShaderProgram diffuse_program;
	ShaderProgram shadow_program;
	ShaderProgram diffuse_shadow_program;
    ShaderProgram evaluate_ssao_program;
    ShaderProgram blur_ssao_texture_program;
	ShaderProgram edges_program;

	// ----------------------------------------------------------------------------
	// Variables (Frame Buffers)
	// ----------------------------------------------------------------------------
protected:
	// DIFFUSE PASS
	GLuint diffuse_fbo = 0;
    GLuint diffuse_texture = 0;

	// DEPTH
    GLuint depth_texture = 0;

	// SHADOW MAP
	GLuint shadow_fbo;
    int shadow_tex_size = 1024;
    GLuint shadow_texture;
	glm::mat4 shadow_matrix;

	// VIEW-SPACE DATA
	GLuint position_vs_texture = 0;
	GLuint normal_vs_texture = 0;

	// SSAO
	GLuint ssao_evaluation_fbo = 0;
	GLuint ssao_blurring_fbo = 0;
	GLuint ssao_occlusion_texture = 0;
	GLuint ssao_blurred_occlusion_texture = 0;
	GLuint ssao_samples_ubo = 0;
	GLuint ssao_random_tangent_vs_texture = 0;

	// Outlines
	GLuint edges_fbo;
	GLuint edges_texture;


	// ----------------------------------------------------------------------------
	// Variables (GUI)
	// ----------------------------------------------------------------------------
protected:
	/** The light position set in the GUI. */
	float gui_light_position = glm::radians(360.f);

	/** The constants identifying what can be displayed on the screen. */
	const int DISPLAY_DEFAULT_SCENE = 0;
    const int DISPLAY_DIFFUSE_SCENE = 1;
	const int DISPLAY_SHADOW_MAP = 2;
    const int DISPLAY_DIFFUSE_SHADOWS = 3;
    const int DISPLAY_POSITIONS_VS = 4;
	const int DISPLAY_NORMALS_VS = 5;
	const int DISPLAY_DEPTH = 6;
	const int AMBIENT_OCCLUSION = 7;
	const int DISPLAY_EDGES = 8;
	const int DISPLAY_EXPANDED_EDGES = 9;
	const int DISPLAY_FINAL_IMAGE = 10;
	/** The GUI labels for the constants above. */
	const char* DISPLAY_LABELS[11] = {
        "Default Scene", 
        "Diffuse", 
        "Shadow Map",
        "Diffuse w/ Shadows", 
        "Positions in View Space", 
        "Normals in View Space",
        "Depth", 
        "Ambient Occlusion", 
        "Edges", 
        "Expanded Edges (Corrective Only)", 
        "Final Image"
	};

	/** The flag determining what will be displayed on the screen right now. */
	int what_to_display = DISPLAY_DEFAULT_SCENE;

	/** The flag determining the outline thickness. */
	int outline_thickness = 1;

    /** The flag determining wether AO will be used or not. */
    bool use_ao = true;

	/** The SSAO radius. */
	float ssao_radius = 0.5f;

	// ----------------------------------------------------------------------------
	// Constructors
	// ----------------------------------------------------------------------------
public:
	Application(int initial_width, int initial_height, std::vector<std::string> arguments = {});

	/** Destroys the {@link Application} and releases the allocated resources. */
	virtual ~Application();

	// ----------------------------------------------------------------------------
	// Shaders
	// ----------------------------------------------------------------------------
	/**
	 * {@copydoc PV227Application::compile_shaders}
	 */
	void compile_shaders() override;

	// ----------------------------------------------------------------------------
	// Initialize Scene
	// ----------------------------------------------------------------------------
public:
	/** Prepares the required cameras. */
	void prepare_cameras();

	/** Prepares the required materials. */
	void prepare_materials();

	/** Prepares the required textures. */
	void prepare_textures();

	/** Prepares the lights. */
	void prepare_lights();

	/** Prepares the scene objects. */
	void prepare_scene();

	/** Prepares the frame buffer objects. */
	void prepare_framebuffers();

	/** Resizes the full screen textures match the window. */
	void resize_fullscreen_textures();

	/** Prepares the frame buffer for the diffuse pass. */
	void prepare_for_diffuse_pass();

	/** Renders the shadow map. */
	void render_shadow_map();

	/** Computes the random samples for SSAO. */
	void compute_random_samples();

	/** Computes the random tangent directions for SSAO. */
	void compute_random_tangent_directions();

	/** Evaluates the SSAO texture. */
	void evaluate_ssao() const;

	/** Applies the blur to the SSAO texture. */
	void apply_blur_ssao() const;

	/** Clears the SSAO framebuffers. */
	void clear_ssao_buffers() const;

	/** Computes outlines using Sobel filter */
	void evaluate_edges();

	// ----------------------------------------------------------------------------
	// Update
	// ----------------------------------------------------------------------------
	/**
	 * {@copydoc PV227Application::update}
	 */
	void update(float delta) override;

	// ----------------------------------------------------------------------------
	// Render
	// ----------------------------------------------------------------------------
public:
	/** @copydoc PV227Application::render */
	void render() override;

	/** Renders the whole scene. */
	void render_scene(ShaderProgram& program, bool use_shadows = false);

	/** Shows the precomputed textures. */
	void show_textures() const;

	// ----------------------------------------------------------------------------
	// GUI
	// ----------------------------------------------------------------------------
public:
	/** @copydoc PV227Application::render_ui */
	void render_ui() override;

	// ----------------------------------------------------------------------------
	// Input Events
	// ----------------------------------------------------------------------------
public:
	/** @copydoc PV227Application::on_resize */
	void on_resize(int width, int height) override;
};
