#pragma once
#include "scene/gui/box_container.h"

class MarchingCubesTerrain;
class EditorNode;
class MenuButton;

class MarchingCubesEditor : public VBoxContainer {
	GDCLASS(MarchingCubesEditor, VBoxContainer);

    MarchingCubesTerrain* node = nullptr;
    EditorNode* editor = nullptr;
	MenuButton* options = nullptr;

	enum Menu {
		MENU_OPTION_REGENERATE_MESH,
		MENU_OPTION_RANDOMISE_MESH,
		MENU_OPTION_CLEAR_MESH,
	};

	static void _bind_methods();
	void _menu_option(int p_option);

public:
	HBoxContainer *toolbar = nullptr;

	bool forward_spatial_input_event(Camera* p_camera, const Ref<InputEvent>& p_event);
	void edit(MarchingCubesTerrain* p_marching_cubes);

	MarchingCubesEditor(EditorNode* p_editor);
};