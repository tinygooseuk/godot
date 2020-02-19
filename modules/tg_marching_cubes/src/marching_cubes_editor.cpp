#include "marching_cubes_editor.h"
#include "marching_cubes_terrain.h"

#include "core/os/keyboard.h"
#include "editor/editor_node.h"
#include "editor/plugins/spatial_editor_plugin.h"
#include "scene/gui/menu_button.h"

void MarchingCubesEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_menu_option", "option"), &MarchingCubesEditor::_menu_option);
	ClassDB::bind_method(D_METHOD("_tool_select", "tool"), &MarchingCubesEditor::_tool_select);
}

void MarchingCubesEditor::_menu_option(int p_option) {
	switch (p_option) {
		case MENU_OPTION_REGENERATE_MESH:
			node->reallocate_memory();
			node->generate_mesh();
			break;

		case MENU_OPTION_RANDOMISE_MESH:
			node->fill_with_noise();
			_menu_option(MENU_OPTION_REGENERATE_MESH); // Now trigger re-gen
			break;

		case MENU_OPTION_CLEAR_MESH:
			node->clear_mesh();
			_menu_option(MENU_OPTION_REGENERATE_MESH); // Now trigger re-gen
			break;
	}
}

void MarchingCubesEditor::_tool_select(int p_tool) {
	tool = p_tool;

	ToolButton* buttons[] = { tool_none, tool_sphere, tool_cube, tool_ruffle };
	const int num_buttons = sizeof(buttons) / sizeof(*buttons);
	
	for (int i = 0; i < num_buttons; i++) {
		buttons[i]->set_pressed(i == p_tool);
	}
}

bool MarchingCubesEditor::forward_spatial_input_event(Camera* p_camera, const Ref<InputEvent>& p_event) {
    if (!node) return false;
	if (tool == TOOL_NONE) return false;

	Ref<InputEventMouse> mouse_event = p_event;
	if (mouse_event.is_valid()) {
		
	}

	return true;
}
void MarchingCubesEditor::edit(MarchingCubesTerrain* p_marching_cubes) {
	node = p_marching_cubes;
}

MarchingCubesEditor::MarchingCubesEditor(EditorNode *p_editor) {
    editor = p_editor;

	toolbar = memnew(HBoxContainer);
	toolbar->set_h_size_flags(SIZE_EXPAND_FILL);
	toolbar->set_alignment(BoxContainer::ALIGN_END);
	SpatialEditor::get_singleton()->add_control_to_menu_panel(toolbar);

    // Set up toolbar
	options = memnew(MenuButton);
	toolbar->add_child(options);

	options->set_text(TTR("Marching Cubes"));
	options->get_popup()->add_item(TTR("Regenerate Mesh"), MENU_OPTION_REGENERATE_MESH);
	options->get_popup()->add_item(TTR("Randomise Mesh"), MENU_OPTION_RANDOMISE_MESH);
	options->get_popup()->add_item(TTR("Clear Mesh"), MENU_OPTION_CLEAR_MESH);

	options->get_popup()->connect("id_pressed", this, "_menu_option");

	toolbar->add_child(memnew(VSeparator));

	// Tools
	tool_none = memnew(ToolButton);
	tool_none->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_none", TTR("Camera Tool"), KEY_Z));
	tool_none->connect("pressed", this, "_tool_select", make_binds(TOOL_NONE));
	tool_none->set_toggle_mode(true);
	toolbar->add_child(tool_none);

	tool_sphere = memnew(ToolButton);
	tool_sphere->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_sphere", TTR("Sphere Tool"), KEY_X));
	tool_sphere->connect("pressed", this, "_tool_select", make_binds(TOOL_SPHERE));
	tool_sphere->set_toggle_mode(true);
	toolbar->add_child(tool_sphere);

	tool_cube = memnew(ToolButton);
	tool_cube->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_cube", TTR("Cube Tool"), KEY_C));
	tool_cube->connect("pressed", this, "_tool_select", make_binds(TOOL_CUBE));
	tool_cube->set_toggle_mode(true);
	toolbar->add_child(tool_cube);

	tool_ruffle = memnew(ToolButton);
	tool_ruffle->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_ruffle", TTR("Ruffle Tool"), KEY_V));
	tool_ruffle->connect("pressed", this, "_tool_select", make_binds(TOOL_RUFFLE));
	tool_ruffle->set_toggle_mode(true);
	toolbar->add_child(tool_ruffle);

	toolbar->hide();

	// Set up palette (self)
	// Palette

	// Final setup
	_tool_select(TOOL_NONE);
}