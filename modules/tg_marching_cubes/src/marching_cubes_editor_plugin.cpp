#include "marching_cubes_editor_plugin.h"
#include "editor/editor_scale.h"
#include "editor/editor_settings.h"
#include "editor/plugins/node_3d_editor_plugin.h"
#include "scene/3d/camera_3d.h"

#include "core/math/geometry.h"
#include "core/os/keyboard.h"

#if TOOLS_ENABLED
void MarchingCubesEditorPlugin::_notification(int p_what) {
	if (p_what == EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED) {

		switch ((int)EditorSettings::get_singleton()->get("editors/marching_cubes/editor_side")) {
			case 0: { // Left.
				Node3DEditor::get_singleton()->get_palette_split()->move_child(marching_cubes_editor, 0);
			} break;
			case 1: { // Right.
				Node3DEditor::get_singleton()->get_palette_split()->move_child(marching_cubes_editor, 1);
			} break;
		}
	}
}

void MarchingCubesEditorPlugin::edit(Object *p_object) {
	marching_cubes_editor->edit(Object::cast_to<MarchingCubesTerrain3D>(p_object));
}

bool MarchingCubesEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class("MarchingCubesTerrain");
}

void MarchingCubesEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		marching_cubes_editor->show();
		marching_cubes_editor->toolbar->show();
		marching_cubes_editor->set_process(true);
		marching_cubes_editor->recreate_gizmo();
		marching_cubes_editor->create_editor_grid();
	} else {
		marching_cubes_editor->toolbar->hide();
		marching_cubes_editor->hide();
		marching_cubes_editor->edit(NULL);
		marching_cubes_editor->set_process(false);
		marching_cubes_editor->free_gizmo();
		marching_cubes_editor->free_editor_grid();
	}
}

MarchingCubesEditorPlugin::MarchingCubesEditorPlugin(EditorNode *p_node) {
	editor = p_node;

	EDITOR_DEF("editors/marching_cubes/editor_side", 1);
	EditorSettings::get_singleton()->add_property_hint(PropertyInfo(Variant::INT, "editors/marching_cubes/editor_side", PROPERTY_HINT_ENUM, "Left,Right"));

	marching_cubes_editor = memnew(MarchingCubesEditor(editor));
	switch ((int)EditorSettings::get_singleton()->get("editors/marching_cubes/editor_side")) {
		case 0: { // Left.
			add_control_to_container(CONTAINER_SPATIAL_EDITOR_SIDE_LEFT, marching_cubes_editor);
		} break;
		case 1: { // Right.
			add_control_to_container(CONTAINER_SPATIAL_EDITOR_SIDE_RIGHT, marching_cubes_editor);
		} break;
	}
	marching_cubes_editor->hide();
}

MarchingCubesEditorPlugin::~MarchingCubesEditorPlugin() {
}
#endif // TOOLS_ENABLED