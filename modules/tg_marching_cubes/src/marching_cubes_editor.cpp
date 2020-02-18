#include "marching_cubes_editor.h"
#include "marching_cubes_terrain.h"

#include "core/os/keyboard.h"
#include "editor/editor_node.h"
#include "editor/plugins/spatial_editor_plugin.h"
#include "scene/gui/menu_button.h"

void MarchingCubesEditor::_bind_methods() {
	ClassDB::bind_method("_menu_option", &MarchingCubesEditor::_menu_option);
}

void MarchingCubesEditor::_menu_option(int p_option) {
	print_line("Clicked " + String::num(p_option));

	switch (p_option) {
		case MENU_OPTION_REGENERATE_MESH:
			//TODO: undo/redo!
			node->reallocate_memory();
			node->generate_mesh();
			break;

		case MENU_OPTION_RANDOMISE_MESH:
			//TODO: undo/redo!
			node->fill_with_noise();
			_menu_option(MENU_OPTION_REGENERATE_MESH); // Now trigger re-gen
			break;

		case MENU_OPTION_CLEAR_MESH:
			//TODO: undo/redo!
			//TODO:
			break;
	}
}

bool MarchingCubesEditor::forward_spatial_input_event(Camera* p_camera, const Ref<InputEvent>& p_event) {
    if (!node) return false;

	return false;
}
void MarchingCubesEditor::edit(MarchingCubesTerrain* p_marching_cubes) {
	node = p_marching_cubes;
}

MarchingCubesEditor::MarchingCubesEditor(EditorNode *p_editor) {
    editor = p_editor;

	toolbar = memnew(HBoxContainer);
	toolbar->set_h_size_flags(SIZE_EXPAND_FILL);
	toolbar->set_alignment(BoxContainer::ALIGN_BEGIN);
	SpatialEditor::get_singleton()->add_control_to_menu_panel(toolbar);

    // Set up toolbar
	toolbar->add_child(memnew(VSeparator));

	options = memnew(MenuButton);
	toolbar->add_child(options);

	options->set_text(TTR("Marching Cubes"));
	options->get_popup()->add_item(TTR("Regenerate Mesh"), MENU_OPTION_REGENERATE_MESH);
	options->get_popup()->add_item(TTR("Randomise Mesh"), MENU_OPTION_RANDOMISE_MESH);
	options->get_popup()->add_item(TTR("Clear Mesh"), MENU_OPTION_CLEAR_MESH);

	options->get_popup()->connect("id_pressed", this, "_menu_option");

	//TODO: add Cube & sphere tools
	toolbar->hide();

	// Set up palette (self)
	// Palette
}