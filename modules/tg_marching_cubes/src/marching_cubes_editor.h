#pragma once
#include "scene/gui/box_container.h"

class MarchingCubesTerrain;
class EditorNode;
class MenuButton;
class ToolButton;
class CheckBox;
class SpinBox;
class HSlider;

class MarchingCubesEditor : public VBoxContainer {
	GDCLASS(MarchingCubesEditor, VBoxContainer);

	// Main refs.
    MarchingCubesTerrain* node = nullptr;
    EditorNode* editor = nullptr;
	MenuButton* options = nullptr;

	enum Menu {
		MENU_OPTION_REGENERATE_MESH,
		MENU_OPTION_RANDOMISE_MESH,
		MENU_OPTION_CLEAR_MESH,
	};

	enum Tool {
		TOOL_NONE,
		TOOL_SPHERE,
		TOOL_CUBE,
		TOOL_RUFFLE
	};
	int tool = TOOL_NONE;

	// Lifecycle
	static void _bind_methods();
	void _notification(int p_what);
	void enter_tree();
	void process(float delta);

	// Actions
	void menu_option(int p_option);
	void tool_select(int p_tool);	
	void update_palette_labels(float new_value = 0.0f);

	// Toolbar Button
	ToolButton* tool_none;
	ToolButton* tool_sphere;
	ToolButton* tool_cube;
	ToolButton* tool_ruffle;

	// Palette widgets
	CheckBox* is_additive;
	Label* power_label;
	HSlider* power_slider;
	Label* radius_label;
	HSlider* radius_slider;

	// State
	Camera* editor_camera = nullptr;
	int mouse_button_down = 0;

	// Gizmo
	void set_gizmo_position(const Vector3& centre, bool is_visible);
	void recreate_gizmo();
	void create_sphere_gizmo(float radius);
	void create_cube_gizmo(const Vector3& extents);

	// Tools
	void brush_cube(const Vector3& centre, float radius, float power, bool additive = true);
	void ruffle_cube(const Vector3& centre, float radius, float power);

public:
	HBoxContainer *toolbar = nullptr;

	bool forward_spatial_input_event(Camera* p_camera, const Ref<InputEvent>& p_event);
	void edit(MarchingCubesTerrain* p_marching_cubes);

	MarchingCubesEditor(EditorNode* p_editor);
};