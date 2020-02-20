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
	void _enter_tree();
	void _menu_option(int p_option);
	void _tool_select(int p_tool);	

	// Toolbar Buttons
	ToolButton* tool_none;
	ToolButton* tool_sphere;
	ToolButton* tool_cube;
	ToolButton* tool_ruffle;

	// Palette widgets
	CheckBox* is_additive;
	HSlider* power_slider;
	HSlider* radius_slider;

	// Tools
	void brush_cube(const Vector3& centre, float radius, float power, bool additive = true);

public:
	HBoxContainer *toolbar = nullptr;

	bool forward_spatial_input_event(Camera* p_camera, const Ref<InputEvent>& p_event);
	void edit(MarchingCubesTerrain* p_marching_cubes);

	MarchingCubesEditor(EditorNode* p_editor);
};