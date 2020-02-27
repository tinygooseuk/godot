#pragma once
#include "scene/gui/box_container.h"

class MarchingCubesTerrain;
class EditorNode;
class MenuButton;
class ToolButton;
class CheckBox;
class SpinBox;
class HSlider;

#if TOOLS_ENABLED
class MarchingCubesEditor : public VBoxContainer {
	GDCLASS(MarchingCubesEditor, VBoxContainer);

	// Main refs.
    MarchingCubesTerrain* node = nullptr;
    EditorNode* editor = nullptr;
	MenuButton* options = nullptr;

	enum Menu {
		MENU_OPTION_REGENERATE_MESH,
		MENU_OPTION_RANDOMISE_MESH,
		MENU_OPTION_INVERT_DATA,
		MENU_OPTION_CLEAR_MESH,
	};

	enum Tool {
		TOOL_CUBE,
		TOOL_SPHERE,
		TOOL_FLATTEN,
		TOOL_RUFFLE,
	};
	int tool = TOOL_CUBE;

	enum Axis {
		AXIS_NONE,
		AXIS_X,
		AXIS_Y,
		AXIS_Z,

		AXIS_Count,
	};
	int axis = AXIS_NONE;
	float axis_level = 0.0f;

	// Lifecycle
	static void _bind_methods();
	void _notification(int p_what);
	void enter_tree();
	void process(float delta);

	// Actions
	void menu_option(int p_option);
	void tool_select(int p_tool);	
	void update_status();
	void update_tool_position();
	void update_gizmo();
	void update_palette_labels(float new_value = 0.0f);

	// Toolbar Buttons
	ToolButton* tool_cube;
	ToolButton *tool_sphere;
	ToolButton* tool_flatten;	
	ToolButton* tool_ruffle;	

	// Palette widgets
	Label* status;

	CheckBox* is_additive;
	Label* power_label;
	HSlider* power_slider;
	Label* radius_label;
	HSlider* radius_slider;

	// State
	Camera* editor_camera = nullptr;
	Vector3 tool_position;
	int mouse_button_down = 0;
	bool shift = false;
	bool editor_passthru = false;
	RID debug_gizmo;
	RID editor_grid;

	// Gizmos
	void recreate_gizmo();
	void create_sphere_gizmo();
	void create_cube_gizmo();
	void create_editor_grid();

	void free_gizmo();
	void free_editor_grid();


	// Tools
	void brush_cube(const Vector3& centre, float radius, float power, bool additive = true);
	void brush_sphere(const Vector3& centre, float radius, float power, bool additive = true);
	void flatten_cube(const Vector3& centre, float radius, float power);
	void ruffle_cube(const Vector3& centre, float radius, float power);

	// Misc
	float get_max_value(Axis axis) const;

public:
	HBoxContainer *toolbar = nullptr;

	
	bool forward_spatial_input_event(Camera* p_camera, const Ref<InputEvent>& p_event);
	void edit(MarchingCubesTerrain* p_marching_cubes);

	MarchingCubesEditor(EditorNode* p_editor);

	friend class MarchingCubesEditorPlugin;
};
#endif // TOOLS_ENABLED