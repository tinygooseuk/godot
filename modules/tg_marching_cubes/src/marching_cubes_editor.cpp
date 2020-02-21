#include "marching_cubes_editor.h"
#include "marching_cubes_terrain.h"

#include "core/os/keyboard.h"
#include "editor/editor_node.h"
#include "editor/plugins/spatial_editor_plugin.h"
#include "scene/gui/menu_button.h"
#include "scene/main/viewport.h"
#include "scene/3d/camera.h"

void MarchingCubesEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("menu_option", "option"), &MarchingCubesEditor::menu_option);
	ClassDB::bind_method(D_METHOD("tool_select", "tool"), &MarchingCubesEditor::tool_select);
	ClassDB::bind_method(D_METHOD("update_palette_labels", "new_value"), &MarchingCubesEditor::update_palette_labels);
}

void MarchingCubesEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: 	enter_tree(); break;
		case NOTIFICATION_PROCESS:		process(get_process_delta_time()); break;
	}

}

void MarchingCubesEditor::enter_tree() {
	tool_none->set_icon(get_icon("Camera2D", "EditorIcons"));
	tool_sphere->set_icon(get_icon("SphereShape", "EditorIcons"));
	tool_cube->set_icon(get_icon("BoxShape", "EditorIcons"));
	tool_ruffle->set_icon(get_icon("SpriteSheet", "EditorIcons"));
}

void MarchingCubesEditor::process(float delta) {
	if (tool > TOOL_NONE && mouse_button_down > 0) {
		Viewport* editor_vp = editor_camera->get_viewport();
		Vector2 mouse_pos = editor_vp->get_mouse_position();

		Vector3 ray_origin = editor_camera->project_ray_origin(mouse_pos);
		Vector3 ray_dir = editor_camera->project_ray_normal(mouse_pos);

		auto* space = editor_camera->get_world()->get_direct_space_state();

		PhysicsDirectSpaceState::RayResult ray;
		bool is_hit = space->intersect_ray(ray_origin, ray_origin + ray_dir * 500.0f, ray);
		if (is_hit) {
			Vector3 grid_pos = node->get_world_position_from_grid_coordinates(node->get_grid_coordinates_from_world_position(ray.position));
			set_gizmo_position(grid_pos, true);

			float radius = radius_slider->get_value();
			float power = power_slider->get_value();
			bool use_additive = is_additive->is_pressed();

			switch (tool) {
				case TOOL_CUBE:
					if (use_additive) {
						brush_cube(ray.position, radius, mouse_button_down == BUTTON_LEFT ? -power : +power, true);
					} else {
						brush_cube(ray.position, radius, mouse_button_down == BUTTON_LEFT ? -1.0f : power, false);
					}
					break;
				case TOOL_SPHERE: break; //TODO:
				case TOOL_RUFFLE:
					ruffle_cube(ray.position, radius, power);
			}
						
		} else {
			set_gizmo_position(Vector3(), false);
		}
	}
}

void MarchingCubesEditor::menu_option(int p_option) {
	switch (p_option) {
		case MENU_OPTION_REGENERATE_MESH:
			node->reallocate_memory();
			node->generate_mesh();
			break;

		case MENU_OPTION_RANDOMISE_MESH:
			node->fill_with_noise();
			menu_option(MENU_OPTION_REGENERATE_MESH); // Now trigger re-gen
			break;

		case MENU_OPTION_CLEAR_MESH:
			node->clear_mesh();
			menu_option(MENU_OPTION_REGENERATE_MESH); // Now trigger re-gen
			break;
	}
}

void MarchingCubesEditor::tool_select(int p_tool) {
	tool = p_tool;

	ToolButton* buttons[] = { tool_none, tool_sphere, tool_cube, tool_ruffle };
	const int num_buttons = sizeof(buttons) / sizeof(*buttons);
	
	for (int i = 0; i < num_buttons; i++) {
		buttons[i]->set_pressed(i == p_tool);
	}

	recreate_gizmo();
}

void MarchingCubesEditor::update_palette_labels(float /*new_value*/) {
	power_label->set_text(TTR("Power") + " (" + String::num(power_slider->get_value(), 2) + ")");
	radius_label->set_text(TTR("Radius") + " (" + String::num(radius_slider->get_value(), 2) + ")");
}

//------------------------------ GIZMOS -------------------------
void MarchingCubesEditor::set_gizmo_position(const Vector3& centre, bool is_visible) {

}
void MarchingCubesEditor::recreate_gizmo() {
	switch (tool) {
		case TOOL_NONE: break;
		case TOOL_SPHERE:
		case TOOL_RUFFLE:
			 create_sphere_gizmo(radius_slider->get_value()); break;
		case TOOL_CUBE:
			break; //TODO:
	}
}
void MarchingCubesEditor::create_sphere_gizmo(float radius) {

}
void MarchingCubesEditor::create_cube_gizmo(const Vector3& extents) {

}


//------------------------------ TOOLS -------------------------
void MarchingCubesEditor::brush_cube(const Vector3& centre, float radius, float power, bool additive) {
	ERR_FAIL_COND(!node);

	int half_range = (int)Math::ceil(radius / node->get_mesh_scale());

	for (int x = -half_range; x <= +half_range; x++) {
		for (int y = -half_range; y <= +half_range; y++) {
			for (int z = -half_range; z <= +half_range; z++) {
				Vector3 offset = Vector3(x, y, z) * node->get_mesh_scale();
				Vector3 coord = node->get_grid_coordinates_from_world_position(centre + offset);	
				
				if (node->are_grid_coordinates_valid(coord))
				{
					float currentValue = additive ? node->get_value_at(coord) : 0.0f;
					node->set_value_at(coord, currentValue + power);
				}
			}	
		}	
	}
	
	node->generate_mesh();
}
void MarchingCubesEditor::ruffle_cube(const Vector3& centre, float radius, float power) {
	ERR_FAIL_COND(!node);

	int half_range = (int)Math::ceil(radius / node->get_mesh_scale());

	for (int x = -half_range; x <= +half_range; x++) {
		for (int y = -half_range; y <= +half_range; y++) {
			for (int z = -half_range; z <= +half_range; z++) {
				Vector3 offset = Vector3(x, y, z) * node->get_mesh_scale();
				Vector3 coord = node->get_grid_coordinates_from_world_position(centre + offset);	
				
				if (node->are_grid_coordinates_valid(coord))
				{
					float random_power = Math::random(-power, +power);
					node->set_value_at(coord, node->get_value_at(coord) + random_power);
				}
			}	
		}	
	}
	
	node->generate_mesh();
}

bool MarchingCubesEditor::forward_spatial_input_event(Camera* p_camera, const Ref<InputEvent>& p_event) {
    if (!node) return false;
	if (tool == TOOL_NONE) return false;

	editor_camera = p_camera;

	Ref<InputEventMouseButton> mouse_button_event = p_event;
	if (mouse_button_event.is_valid()) {
		UndoRedo* undo_redo = editor->get_undo_redo();

		if (mouse_button_event->is_pressed()) {
			if (mouse_button_down == 0) {
				// Begin transaction
				undo_redo->create_action("Editing Marching Cubes");
				//TODO: add undo and redo events here to support undo properly
			}
			mouse_button_down = mouse_button_event->get_button_index();
		} else {
			if (mouse_button_down > 0 && undo_redo->is_committing_action()) {
				undo_redo->commit_action();
			}
			mouse_button_down = 0;
		}
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

	options->get_popup()->connect("id_pressed", this, "menu_option");

	toolbar->add_child(memnew(VSeparator));

	// Tools
	tool_none = memnew(ToolButton);
	tool_none->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_none", TTR("Camera Tool"), KEY_Z));
	tool_none->connect("pressed", this, "tool_select", make_binds(TOOL_NONE));
	tool_none->set_toggle_mode(true);
	toolbar->add_child(tool_none);

	tool_sphere = memnew(ToolButton);
	tool_sphere->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_sphere", TTR("Sphere Tool"), KEY_X));
	tool_sphere->connect("pressed", this, "tool_select", make_binds(TOOL_SPHERE));
	tool_sphere->set_toggle_mode(true);
	toolbar->add_child(tool_sphere);

	tool_cube = memnew(ToolButton);
	tool_cube->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_cube", TTR("Cube Tool"), KEY_C));
	tool_cube->connect("pressed", this, "tool_select", make_binds(TOOL_CUBE));
	tool_cube->set_toggle_mode(true);
	toolbar->add_child(tool_cube);

	tool_ruffle = memnew(ToolButton);
	tool_ruffle->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_ruffle", TTR("Ruffle Tool"), KEY_V));
	tool_ruffle->connect("pressed", this, "tool_select", make_binds(TOOL_RUFFLE));
	tool_ruffle->set_toggle_mode(true);
	toolbar->add_child(tool_ruffle);

	toolbar->hide();

	// Set up palette (self)
	HBoxContainer* radius_box = memnew(HBoxContainer);
	radius_box->set_alignment(AlignMode::ALIGN_BEGIN);
	radius_box->set_h_size_flags(SIZE_EXPAND_FILL);
	{
		radius_label = memnew(Label);
		radius_label->set_text(TTR("Radius"));
		radius_label->set_custom_minimum_size(Size2(100.0f, 0.0f));
		radius_box->add_child(radius_label);

		radius_slider = memnew(HSlider);
		radius_slider->set_custom_minimum_size(Size2(200.0f, 0.0f));
		radius_slider->set_min(0.01f);
		radius_slider->set_max(10.0f);
		radius_slider->set_step(0.25f);
		radius_slider->set_value(2.0f);
		radius_slider->connect("value_changed", this, "update_palette_labels");
		radius_box->add_child(radius_slider);
	}
	add_child(radius_box);

	HBoxContainer* power_box = memnew(HBoxContainer);
	power_box->set_alignment(AlignMode::ALIGN_BEGIN);
	power_box->set_h_size_flags(SIZE_EXPAND_FILL);
	{
		power_label = memnew(Label);
		power_label->set_text(TTR("Power"));
		power_label->set_custom_minimum_size(Size2(100.0f, 0.0f));
		power_box->add_child(power_label);
		
		power_slider = memnew(HSlider);
		power_slider->set_custom_minimum_size(Size2(200.0f, 0.0f));
		power_slider->set_min(0.001f);
		power_slider->set_max(1.000f);
		power_slider->set_step(0.001f);
		power_slider->set_value(0.01f);
		power_slider->connect("value_changed", this, "update_palette_labels");
		power_box->add_child(power_slider);
	}
	add_child(power_box);

	is_additive = memnew(CheckBox);
	is_additive->set_text(TTR("Is Additive?"));
	add_child(is_additive);

	// Final setup
	tool_select(TOOL_NONE);
	update_palette_labels();
}