#include "marching_cubes_editor.h"
#include "marching_cubes_terrain.h"

#include "core/os/keyboard.h"
#include "editor/editor_node.h"
#include "editor/plugins/spatial_editor_plugin.h"
#include "scene/3d/camera.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/color_rect.h"
#include "scene/main/viewport.h"
#include "scene/resources/box_shape.h"
#include "scene/resources/mesh_data_tool.h"
#include "scene/resources/primitive_meshes.h"
#include "scene/resources/sphere_shape.h"

#if TOOLS_ENABLED

#define _USE_MATH_DEFINES
#include <cmath>

void MarchingCubesEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("menu_option", "option"), &MarchingCubesEditor::menu_option);
	ClassDB::bind_method(D_METHOD("tool_select", "tool"), &MarchingCubesEditor::tool_select);
	ClassDB::bind_method(D_METHOD("update_palette_labels", "new_value"), &MarchingCubesEditor::update_palette_labels);
	ClassDB::bind_method(D_METHOD("bump_data", "direction"), &MarchingCubesEditor::bump_data);
	ClassDB::bind_method(D_METHOD("apply_data", "data"), &MarchingCubesEditor::apply_data);
	ClassDB::bind_method(D_METHOD("change_colour", "delta"), &MarchingCubesEditor::change_colour);
}

void MarchingCubesEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: 	enter_tree(); break;
		case NOTIFICATION_PROCESS:		process(get_process_delta_time()); break;
	}
}

void MarchingCubesEditor::enter_tree() {
	tool_cube->set_icon(get_icon("BoxShape", "EditorIcons"));
	tool_sphere->set_icon(get_icon("SphereShape", "EditorIcons"));
	tool_paint->set_icon(get_icon("CanvasItem", "EditorIcons"));
	tool_flatten->set_icon(get_icon("PlaneShape", "EditorIcons"));
	tool_ruffle->set_icon(get_icon("SpriteSheet", "EditorIcons"));
}

void MarchingCubesEditor::process(float delta) {
	if (mouse_button_down > 0) {
		float radius = radius_slider->get_value();
		float power = power_slider->get_value();
		bool use_additive = is_additive->is_pressed();

		switch (tool) {
			case TOOL_CUBE:
				if (use_additive) {
					brush_cube(tool_position, radius, shift ? +power : -power, true);
				} else {
					brush_cube(tool_position, radius, shift ? mouse_button_down : -1.0f, false);
				}
				break;
			case TOOL_SPHERE:
				if (use_additive) {
					brush_sphere(tool_position, radius, shift ? +power : -power, true);
				} else {
					brush_sphere(tool_position, radius, shift ? mouse_button_down : -1.0f, false);
				}
				break;
			case TOOL_PAINT:
				paint_sphere(tool_position, radius, colour);
				break;
			case TOOL_FLATTEN:
				flatten_cube(tool_position, radius, power);
				break;
			case TOOL_RUFFLE:
				ruffle_cube(tool_position, radius, power);
				break;
		}
	}
}

void MarchingCubesEditor::menu_option(int p_option) {
	ERR_FAIL_COND(!node);

	switch (p_option) {
		case MENU_OPTION_REGENERATE_MESH:
			node->reallocate_memory();
			node->generate_mesh();
			break;

		case MENU_OPTION_RANDOMISE_MESH:
			node->fill_with_noise();
			break;

		case MENU_OPTION_INVERT_DATA:
			node->invert_data_sign();
			break;

		case MENU_OPTION_BAKE_TO_MESHINSTANCE: {
			node->generate_mesh();
			
			Ref<Mesh> old_mesh = node->get_mesh();
			
			// Calculate the midpoint of all surfaces of the mesh
			Vector3 avg_vertex_all_surf;
			for (int surf_idx = 0; surf_idx < old_mesh->get_surface_count(); surf_idx++) {
				Vector3 avg_vertex;

				MeshDataTool mdt;
				mdt.create_from_surface(old_mesh, surf_idx);

				for (int v = 0; v < mdt.get_vertex_count(); v++) {
					avg_vertex += mdt.get_vertex(v);
				}
				avg_vertex /= mdt.get_vertex_count();

				avg_vertex_all_surf += avg_vertex;
			}
			avg_vertex_all_surf /= old_mesh->get_surface_count();

			// Create a new mesh
			Ref<ArrayMesh> new_mesh = memnew(ArrayMesh);

			for (int surf_idx = 0; surf_idx < old_mesh->get_surface_count(); surf_idx++) {
				MeshDataTool mdt;
				mdt.create_from_surface(old_mesh, surf_idx);

				for (int v = 0; v < mdt.get_vertex_count(); v++) {
					mdt.set_vertex(v, mdt.get_vertex(v) - avg_vertex_all_surf);
				}
				
				mdt.commit_to_surface(new_mesh);
			}

			// Finally save the mesh
			MeshInstance *mesh_instance = memnew(MeshInstance);
			mesh_instance->set_name((String)get_name() + " (Baked)");
			mesh_instance->set_mesh(new_mesh);

			UndoRedo *ur = EditorNode::get_singleton()->get_undo_redo();
			ur->create_action(TTR("Bake to MeshInstance"));

			ur->add_do_method(node->get_parent(), "add_child", mesh_instance);
			ur->add_do_method(node->get_parent(), "move_child", mesh_instance, node->get_index() + 1);

			ur->add_do_method(mesh_instance, "set_owner", node->get_parent()->get_owner());
			ur->add_do_reference(mesh_instance);
			ur->add_undo_method(node, "remove_child", mesh_instance);
			ur->commit_action();
		} break;

		case MENU_OPTION_CLEAR_MESH:
			node->clear_mesh();
			break;
	}

	if (p_option != MENU_OPTION_REGENERATE_MESH) {
		menu_option(MENU_OPTION_REGENERATE_MESH); // Now trigger re-gen
	}
}

void MarchingCubesEditor::tool_select(int p_tool) {
	tool = p_tool;

	ToolButton *buttons[] = { tool_cube, tool_sphere, tool_paint, tool_flatten, tool_ruffle };
	const int num_buttons = sizeof(buttons) / sizeof(*buttons);
	
	for (int i = 0; i < num_buttons; i++) {
		buttons[i]->set_pressed(i == p_tool);
	}

	recreate_gizmo();
	update_status();
}

void MarchingCubesEditor::update_status() {
	String status_text;

	switch (tool) {
		case TOOL_SPHERE: status_text = "Sphere tool"; break;
		case TOOL_CUBE: status_text = "Cube tool"; break;
		case TOOL_PAINT: status_text = "Paint tool"; break;
		case TOOL_FLATTEN: status_text = "Flatten tool"; break;
		case TOOL_RUFFLE: status_text = "Ruffle tool"; break;
	}

	switch (axis) {
		case AXIS_NONE: status_text += " - Freeform Edit"; break;
		case AXIS_X: status_text += " - X Axis @ " + String::num(axis_level); break;
		case AXIS_Y: status_text += " - Y Axis @ " + String::num(axis_level); break;
		case AXIS_Z: status_text += " - Z Axis @ " + String::num(axis_level); break;
	}
	status_text += "\n\nMiddle-Mouse or Space: Switch Axis";
	status_text += "\nMouse wheel or -/+: Change Level";

	status->set_text(status_text);
}

void MarchingCubesEditor::update_tool_position() {
	Viewport *editor_vp = editor_camera->get_viewport();
	const Vector2 mouse_pos = editor_vp->get_mouse_position();

	const Vector3 ray_origin = editor_camera->project_ray_origin(mouse_pos);
	const Vector3 ray_dir = editor_camera->project_ray_normal(mouse_pos);

	// Centre position = node origin + half size + half tile offset on XZ
	const Vector3 centre_position = node->get_global_transform().origin +
		Vector3(node->terrain_data->width * node->mesh_scale, node->terrain_data->height * node->mesh_scale, node->terrain_data->depth * node->mesh_scale) * 0.5f +
		Vector3(node->mesh_scale, 0.0f, node->mesh_scale) * 0.5f;

	if (!node) return;

	// Move tool position
	switch (axis) {
		case AXIS_NONE: {
			auto *space = editor_camera->get_world()->get_direct_space_state();

			PhysicsDirectSpaceState::RayResult ray;
			bool is_hit = space->intersect_ray(ray_origin, ray_origin + ray_dir * 500.0f, ray);
			if (is_hit) {
				tool_position = node->get_world_position_from_grid_coordinates(node->get_grid_coordinates_from_world_position(ray.position));
			}

			if (editor_grid.is_valid()) {
				VS::get_singleton()->instance_set_visible(editor_grid, false);
			}
		} break;
		case AXIS_X: {
			Plane p = Plane(Vector3(axis_level * node->mesh_scale, 0.0f, 0.0f), Vector3(1.0, 0.0f, 0.0f));

			Vector3 intersection;
			if (p.intersects_ray(ray_origin, ray_dir, &intersection)) {
				tool_position = node->get_world_position_from_grid_coordinates(node->get_grid_coordinates_from_world_position(intersection));
			}

			if (editor_grid.is_valid()) {
				Vector3 position = centre_position;
				position.x = axis_level * node->mesh_scale;

				Transform xform;
				xform.set_basis(Basis(Vector3(0.0f, 0.0f, M_PI_2)));
				xform.set_origin(position);

				VS::get_singleton()->instance_set_transform(editor_grid, xform);
				VS::get_singleton()->instance_set_visible(editor_grid, true);
			}
		} break;

		case AXIS_Y: {
			Plane p = Plane(Vector3(0.0f, axis_level * node->mesh_scale, 0.0f), Vector3(0.0, 1.0f, 0.0f));

			Vector3 intersection;
			if (p.intersects_ray(ray_origin, ray_dir, &intersection)) {
				tool_position = node->get_world_position_from_grid_coordinates(node->get_grid_coordinates_from_world_position(intersection));
			}

			if (editor_grid.is_valid()) {
				Vector3 position = centre_position;
				position.y = axis_level * node->mesh_scale;

				Transform xform;
				xform.set_basis(Basis(Vector3(0.0f, 0.0f, 0.0f)));
				xform.set_origin(position);

				VS::get_singleton()->instance_set_transform(editor_grid, xform);
				VS::get_singleton()->instance_set_visible(editor_grid, true);
			}
		} break;

		case AXIS_Z: {
			Plane p = Plane(Vector3(0.0f, 0.0f, axis_level * node->mesh_scale), Vector3(0.0, 0.0f, 1.0f));

			Vector3 intersection;
			if (p.intersects_ray(ray_origin, ray_dir, &intersection)) {
				tool_position = node->get_world_position_from_grid_coordinates(node->get_grid_coordinates_from_world_position(intersection));
			}

			if (editor_grid.is_valid()) {
				Vector3 position = centre_position;
				position.z = axis_level * node->mesh_scale;

				Transform xform;
				xform.set_basis(Basis(Vector3(M_PI_2, 0.0f, 0.0f)));
				xform.set_origin(position);

				VS::get_singleton()->instance_set_transform(editor_grid, xform);
				VS::get_singleton()->instance_set_visible(editor_grid, true);
			}
		} break;
	}

	update_gizmo();
}

void MarchingCubesEditor::update_gizmo() {
	VisualServer *vs = VisualServer::get_singleton();
	if (debug_gizmo.is_valid()) {
		Transform gizmo_transform;
		gizmo_transform.set_origin(tool_position);

		switch (tool) {
			case TOOL_SPHERE: 
			case TOOL_PAINT: 
			case TOOL_RUFFLE: {
				float radius = radius_slider->get_value();
				gizmo_transform.set_basis(Basis().scaled(Vector3(radius, radius, radius) * 2.0f));
			} break;

			case TOOL_CUBE:
			case TOOL_FLATTEN: {
				float radius = radius_slider->get_value();
				gizmo_transform.set_basis(Basis().scaled(Vector3(radius, radius, radius) * 2.0f));
			} break;
		}
		vs->instance_set_transform(debug_gizmo, gizmo_transform);
	}
}

void MarchingCubesEditor::update_palette_labels(float /*new_value*/) {
	radius_label->set_text(TTR("Radius") + " (" + String::num(radius_slider->get_value(), 2) + ")");
	power_label->set_text(TTR("Power") + " (" + String::num(power_slider->get_value(), 4) + ")");
}

//------------------------------ GIZMOS -------------------------
void MarchingCubesEditor::recreate_gizmo() {
	if (!node) return; // Can't create with no node as can't get world 😢

	switch (tool) {
		case TOOL_SPHERE:
		case TOOL_PAINT:
		case TOOL_RUFFLE:
			create_sphere_gizmo();
			break;
		case TOOL_CUBE:
			create_cube_gizmo();
			break;
	}
}

void MarchingCubesEditor::create_sphere_gizmo() {
	VisualServer *vs = VisualServer::get_singleton();
	
	// Remove existing
	free_gizmo();
	
	// New gizmo
	SphereShape *shape = memnew(SphereShape);
	shape->set_radius(1.0f);

	RID scenario = node->get_world()->get_scenario();
	RID cube_rid = shape->get_debug_mesh()->get_rid(); //TODO: free this too?

	debug_gizmo = vs->instance_create2(cube_rid, scenario);
}

void MarchingCubesEditor::create_cube_gizmo() {
	VisualServer *vs = VisualServer::get_singleton();

	// Remove existing
	free_gizmo();

	// New gizmo
	BoxShape *shape = memnew(BoxShape);
	shape->set_extents(Vector3(1.0f, 1.0f, 1.0f));

	RID scenario = node->get_world()->get_scenario();
	RID cube_rid = shape->get_debug_mesh()->get_rid(); //TODO: free this too?

	debug_gizmo = vs->instance_create2(cube_rid, scenario);
}

void MarchingCubesEditor::create_editor_grid() {
	if (editor_grid.is_valid()) {
		VS::get_singleton()->free(editor_grid);
	}

	static constexpr int GRID_EXTENTS = 50;
	static constexpr int HALF_GRID_EXTENTS = GRID_EXTENTS / 2;

	PoolVector3Array verts;
	verts.resize((GRID_EXTENTS + 1) * 2 * 2 * 2); // 10 lines of 2 verts on 2 layers in 2 axes
	auto vert_write = verts.write();
	int v_ptr = 0;

	PoolColorArray vert_colours;
	vert_colours.resize((GRID_EXTENTS + 1) * 2 * 2 * 2); // 10 lines of 2 verts on 2 layers in 2 axes
	auto vc_write = vert_colours.write();
	int vc_ptr = 0;

	// Create grid	
	float grid_offset = node->mesh_scale * float(HALF_GRID_EXTENTS);
	float radius = radius_slider->get_value();

	for (int i = 0; i <= GRID_EXTENTS; i++) {
		// Add vertices
		vert_write[v_ptr++] = Vector3(-float(HALF_GRID_EXTENTS) * node->mesh_scale, -node->mesh_scale * radius * 0.5f, float(i * node->mesh_scale) - grid_offset);
		vert_write[v_ptr++] = Vector3(+float(HALF_GRID_EXTENTS) * node->mesh_scale, -node->mesh_scale * radius * 0.5f, float(i * node->mesh_scale) - grid_offset);

		vert_write[v_ptr++] = Vector3(float(i * node->mesh_scale) - grid_offset, -node->mesh_scale * radius * 0.5f, -float(HALF_GRID_EXTENTS) * node->mesh_scale);
		vert_write[v_ptr++] = Vector3(float(i * node->mesh_scale) - grid_offset, -node->mesh_scale * radius * 0.5f, +float(HALF_GRID_EXTENTS) * node->mesh_scale);

		vert_write[v_ptr++] = Vector3(-float(HALF_GRID_EXTENTS) * node->mesh_scale, +node->mesh_scale * radius * 0.5f, float(i * node->mesh_scale) - grid_offset);
		vert_write[v_ptr++] = Vector3(+float(HALF_GRID_EXTENTS) * node->mesh_scale, +node->mesh_scale * radius * 0.5f, float(i * node->mesh_scale) - grid_offset);

		vert_write[v_ptr++] = Vector3(float(i * node->mesh_scale) - grid_offset, +node->mesh_scale * radius * 0.5f, -float(HALF_GRID_EXTENTS) * node->mesh_scale);
		vert_write[v_ptr++] = Vector3(float(i * node->mesh_scale) - grid_offset, +node->mesh_scale * radius * 0.5f, +float(HALF_GRID_EXTENTS) * node->mesh_scale);

		// Add colours
		float distance_to_centre = Math::absf(i - HALF_GRID_EXTENTS) / (float)HALF_GRID_EXTENTS;
		for (int i = 0; i < 8; i++) {
			vc_write[vc_ptr++] = Color(1.0f, 1.0f, 1.0f, Math::lerp(1.0f, 0.2f, distance_to_centre));
		}
	}

	RID scenario = node->get_world()->get_scenario();
	RID grid_rid = VS::get_singleton()->mesh_create(); //TODO: free this one!!

	Array mesh_info;
	mesh_info.resize(ArrayMesh::ARRAY_MAX);
	mesh_info[ArrayMesh::ARRAY_VERTEX] = verts;
	mesh_info[ArrayMesh::ARRAY_COLOR] = vert_colours;

	VS::get_singleton()->mesh_add_surface_from_arrays(grid_rid, VS::PRIMITIVE_LINES, mesh_info);

	SpatialMaterial *spat_mat = memnew(SpatialMaterial);
	spat_mat->set_flag(SpatialMaterial::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	VS::get_singleton()->mesh_surface_set_material(grid_rid, 0, spat_mat->get_rid());

	editor_grid = VS::get_singleton()->instance_create2(grid_rid, scenario);
}

void MarchingCubesEditor::free_gizmo() {
	if (debug_gizmo.is_valid()) {
		VisualServer::get_singleton()->free(debug_gizmo);
		debug_gizmo = {};
	}
}

void MarchingCubesEditor::free_editor_grid() {
	if (editor_grid.is_valid()) {
		VisualServer::get_singleton()->free(editor_grid);
		editor_grid = {};
	}
}

//------------------------------ TOOLS -------------------------
void MarchingCubesEditor::brush_cube(const Vector3 &centre, float radius, float power, bool additive) {
	ERR_FAIL_COND(!node);

	node->brush_cube(centre, radius, power, additive);
	node->generate_mesh();
}
void MarchingCubesEditor::brush_sphere(const Vector3 &centre, float radius, float power, bool additive) {
	ERR_FAIL_COND(!node);

	node->brush_sphere(centre, radius, power, additive);
	node->generate_mesh();
}
void MarchingCubesEditor::paint_sphere(const Vector3 &centre, float radius, int colour) {
	ERR_FAIL_COND(!node);

	node->paint_sphere(centre, radius, colour);
	node->generate_mesh();
}
void MarchingCubesEditor::flatten_cube(const Vector3 &centre, float radius, float power) {
	ERR_FAIL_COND(!node);

	node->flatten_cube(centre, radius, power);	
	node->generate_mesh();
}
void MarchingCubesEditor::ruffle_cube(const Vector3 &centre, float radius, float power) {
	ERR_FAIL_COND(!node);

	node->ruffle_cube(centre, radius, power);	
	node->generate_mesh();
}
void MarchingCubesEditor::bump_data(int direction) {
	Ref<MarchingCubesData> data = memnew(MarchingCubesData);
	data->width = node->terrain_data->width; 
	data->height = node->terrain_data->height;
	data->depth = node->terrain_data->depth;
	data->data.resize(data->width * data->height * data->depth);

	//TODO: memdelete?
	Ref<MarchingCubesData> old_data = node->terrain_data.ptr();

	Vector3 bump_dir = { 0.0f, 0.0f, 0.0f };
	switch (direction) {
		case BUMP_LEFT: 		bump_dir.x = +1; break;
		case BUMP_UP: 			bump_dir.y = -1; break;
		case BUMP_DOWN: 		bump_dir.y = +1; break;
		case BUMP_RIGHT: 		bump_dir.x = -1; break;
		case BUMP_FORWARD: 		bump_dir.z = -1; break;
		case BUMP_BACKWARD: 	bump_dir.z = +1; break;
	}

	auto read = old_data->data.read();
	auto write = data->data.write();
	for (float x = 0; x < data->width; x++) {
		for (float y = 0; y < data->height; y++) {
			for (float z = 0; z < data->depth; z++) {
				const Vector3 coord = Vector3(x, y, z);
				const int index = node->coord_to_index(coord);

				const Vector3 old_coord = coord + bump_dir;
				const int old_index = node->coord_to_index(old_coord);

				write[index] = read[old_index];
			}
		}
	}

	node->terrain_data = data;
	node->generate_mesh();
}

//------------------------------ MISC -------------------------
void MarchingCubesEditor::set_colour(int p_colour_index) {
	if (!node || node->terrain_data->colour_palette.empty()) {
		colour_label->set_text("0");
		colour_rect->set_frame_color(Color(1.0f, 1.0f, 1.0f));
		return;
	}

	colour = clamp(p_colour_index, 0, node->terrain_data->colour_palette.size()-1);

	colour_label->set_text(String::num(colour));
	colour_rect->set_frame_color(node->terrain_data->colour_palette[colour]);
}
void MarchingCubesEditor::change_colour(int p_colour_index_delta) {
	set_colour(colour + p_colour_index_delta);
}
float MarchingCubesEditor::get_max_value(const Axis axis) const {
	constexpr float DEFAULT_VALUE = 256.0f;

	if (!node) {
		return DEFAULT_VALUE;
	}

	switch (axis) {
		case AXIS_X: return node->terrain_data->get_width();
		case AXIS_Y: return node->terrain_data->get_height();
		case AXIS_Z: return node->terrain_data->get_depth();

		default: return DEFAULT_VALUE;
	}
}

void MarchingCubesEditor::begin_editing() {
	copy_data(node->get_terrain_data()->data, stashed_data);
	copy_data(node->get_terrain_data()->colour_data, stashed_colour_data);

	is_editing = true;
}
void MarchingCubesEditor::end_editing() {
	ERR_FAIL_COND(!is_editing);

	UndoRedo *ur = editor->get_undo_redo();
	ur->create_action("Marching Cubes editing");
	ur->add_do_method(this, "apply_data", node->get_terrain_data()->data, node->get_terrain_data()->colour_data);
	ur->add_undo_method(this, "apply_data", stashed_data, stashed_colour_data);
	ur->commit_action();

	is_editing = false;
}

void MarchingCubesEditor::apply_data(const PoolRealArray &p_data, const PoolByteArray &p_colour_data) {
	ERR_FAIL_COND(!node);

	copy_data(p_data, node->get_terrain_data()->data);
	copy_data(p_colour_data, node->get_terrain_data()->colour_data);
	node->generate_mesh();
}


//------------------------------ PUBLIC -------------------------
bool MarchingCubesEditor::forward_spatial_input_event(Camera *p_camera, const Ref<InputEvent> &p_event) {
    if (!node) return false;

	editor_camera = p_camera;

	const float max_axislevel_value = get_max_value((Axis)axis);

	Ref<InputEventMouseButton> mouse_button_event = p_event;
	bool was_passthru = editor_passthru;

	if (mouse_button_event.is_valid() && mouse_button_event->get_button_index() == BUTTON_RIGHT) {
		editor_passthru = mouse_button_event->is_pressed();
	}
	
	if (editor_passthru || was_passthru) {
		return false;
	}

	if (mouse_button_event.is_valid()) {
		if (mouse_button_event->is_pressed()) {
			switch (mouse_button_event->get_button_index()) {
				case BUTTON_WHEEL_DOWN:
					if (axis_level > 0.0f) axis_level -= 1.0f;
					update_tool_position();
					update_status();
					break;
				case BUTTON_WHEEL_UP:
					if (axis_level < max_axislevel_value) axis_level += 1.0f;
					update_tool_position();
					update_status();
					break;
				case BUTTON_MIDDLE:
					axis = (axis + 1) % AXIS_Count;
					update_tool_position();
					update_status();
					break;

				default:
					// Not a wheel event
					mouse_button_down = mouse_button_event->get_button_index();
					if (mouse_button_down == BUTTON_LEFT) {
						begin_editing(); // for undo/redo
					}
					break;
			}
		} else {
			end_editing();
			mouse_button_down = 0;
		}
	}

	Ref<InputEventKey> key_event = p_event;
	if (key_event.is_valid() && key_event->is_pressed() && !key_event->is_echo()) {
		if (key_event->get_scancode() == KEY_MINUS && axis_level > 0.0f) {
			axis_level -= 1.0f;
			update_tool_position();
			update_status();
		} else if (key_event->get_scancode() == KEY_PLUS && axis_level < max_axislevel_value) {
			axis_level += 1.0f;
			update_tool_position();
			update_status();
		} else if (key_event->get_scancode() == KEY_SPACE) {
			axis = (axis + 1) % AXIS_Count;
			update_tool_position();
			update_status();
		}
	}
	if (key_event.is_valid() && key_event->get_scancode() == KEY_SHIFT) {
		shift = key_event->is_pressed();
	}

	Ref<InputEventMouseMotion> mouse_move_event = p_event;
	if (mouse_move_event.is_valid()) {
		update_tool_position();
		update_status();
	}

	return true;
}
void MarchingCubesEditor::edit(MarchingCubesTerrain *p_marching_cubes) {
	node = p_marching_cubes;
	
	if (node && node->terrain_data.is_null()) {
		node->set_terrain_data(memnew(MarchingCubesData));
	}
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
	options->get_popup()->add_item(TTR("Invert Data"), MENU_OPTION_INVERT_DATA);
	options->get_popup()->add_item(TTR("Bake to MeshInstance"), MENU_OPTION_BAKE_TO_MESHINSTANCE);
	options->get_popup()->add_item(TTR("Clear Mesh"), MENU_OPTION_CLEAR_MESH);

	options->get_popup()->connect("id_pressed", this, "menu_option");

	toolbar->add_child(memnew(VSeparator));

	// Tools
	tool_cube = memnew(ToolButton);
	tool_cube->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_cube", TTR("Cube Tool"), KEY_Z));
	tool_cube->connect("pressed", this, "tool_select", make_binds(TOOL_CUBE));
	tool_cube->set_toggle_mode(true);
	toolbar->add_child(tool_cube);

	tool_sphere = memnew(ToolButton);
	tool_sphere->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_sphere", TTR("Sphere Tool"), KEY_X));
	tool_sphere->connect("pressed", this, "tool_select", make_binds(TOOL_SPHERE));
	tool_sphere->set_toggle_mode(true);
	toolbar->add_child(tool_sphere);

	tool_paint = memnew(ToolButton);
	tool_paint->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_paint", TTR("Paint Tool"), KEY_C));
	tool_paint->connect("pressed", this, "tool_select", make_binds(TOOL_PAINT));
	tool_paint->set_toggle_mode(true);
	toolbar->add_child(tool_paint);

	tool_flatten = memnew(ToolButton);
	tool_flatten->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_flatten", TTR("Flatten Tool"), KEY_V));
	tool_flatten->connect("pressed", this, "tool_select", make_binds(TOOL_FLATTEN));
	tool_flatten->set_toggle_mode(true);
	toolbar->add_child(tool_flatten);

	tool_ruffle = memnew(ToolButton);
	tool_ruffle->set_shortcut(ED_SHORTCUT("marching_cubes_editor/tool_ruffle", TTR("Ruffle Tool"), KEY_B));
	tool_ruffle->connect("pressed", this, "tool_select", make_binds(TOOL_RUFFLE));
	tool_ruffle->set_toggle_mode(true);
	toolbar->add_child(tool_ruffle);

	toolbar->hide();

	// Set up palette (self)
	status = memnew(Label);
	add_child(status);

	HBoxContainer *radius_box = memnew(HBoxContainer);
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

	HBoxContainer *power_box = memnew(HBoxContainer);
	power_box->set_alignment(AlignMode::ALIGN_BEGIN);
	power_box->set_h_size_flags(SIZE_EXPAND_FILL);
	{
		power_label = memnew(Label);
		power_label->set_text(TTR("Power"));
		power_label->set_custom_minimum_size(Size2(100.0f, 0.0f));
		power_box->add_child(power_label);
		
		power_slider = memnew(HSlider);
		power_slider->set_custom_minimum_size(Size2(200.0f, 0.0f));
		power_slider->set_exp_ratio(true);
		power_slider->set_min(0.001f);
		power_slider->set_max(1.000f);
		power_slider->set_step(0.001f);
		power_slider->set_value(0.01f);
		power_slider->connect("value_changed", this, "update_palette_labels");
		power_box->add_child(power_slider);
	}
	add_child(power_box);

	is_additive = memnew(CheckBox);
	is_additive->set_pressed(true);
	is_additive->set_text(TTR("Is Additive?"));
	add_child(is_additive);

	add_child(memnew(HSeparator));

	Label *bump_label = memnew(Label);
	bump_label->set_text("Bump:");
	add_child(bump_label);

	HBoxContainer *bump_box = memnew(HBoxContainer);
	bump_box->set_alignment(AlignMode::ALIGN_BEGIN);
	bump_box->set_h_size_flags(SIZE_EXPAND_FILL);
	{ 
		Button *left_bump = memnew(Button);
		left_bump->set_text("Left");
		left_bump->connect("pressed", this, "bump_data", make_binds(BUMP_LEFT));
		bump_box->add_child(left_bump);

		Button *up_bump = memnew(Button);
		up_bump->set_text("Up");
		up_bump->connect("pressed", this, "bump_data", make_binds(BUMP_UP));
		bump_box->add_child(up_bump);

		Button *down_bump = memnew(Button);
		down_bump->set_text("Down");
		down_bump->connect("pressed", this, "bump_data", make_binds(BUMP_DOWN));
		bump_box->add_child(down_bump);
	
		Button *right_bump = memnew(Button);
		right_bump->set_text("Right");
		right_bump->connect("pressed", this, "bump_data", make_binds(BUMP_RIGHT));
		bump_box->add_child(right_bump);
	
		Button *forward_bump = memnew(Button);
		forward_bump->set_text("Forward");
		forward_bump->connect("pressed", this, "bump_data", make_binds(BUMP_FORWARD));
		bump_box->add_child(forward_bump);
	
		Button *back_bump = memnew(Button);
		back_bump->set_text("Back");
		back_bump->connect("pressed", this, "bump_data", make_binds(BUMP_BACKWARD));
		bump_box->add_child(back_bump);
	}
	add_child(bump_box);

	add_child(memnew(HSeparator));

	Label *colour_header_label = memnew(Label);
	colour_header_label->set_text("Colour:");
	add_child(colour_header_label);

	HBoxContainer *colour_box = memnew(HBoxContainer);
	colour_box->set_alignment(AlignMode::ALIGN_BEGIN);
	colour_box->set_h_size_flags(SIZE_EXPAND_FILL);
	{ 
		Button *previous_colour = memnew(Button);
		previous_colour->set_text("<-");
		previous_colour->connect("pressed", this, "change_colour", make_binds(-1));
		previous_colour->set_custom_minimum_size(Size2(40.0f, 40.0f));
		colour_box->add_child(previous_colour);

		Container *overlay = memnew(Container);
		overlay->set_custom_minimum_size(Size2(40.0f, 40.0f));
		{
			colour_rect = memnew(ColorRect);
			colour_rect->set_custom_minimum_size(Size2(40.0f, 40.0f));
			overlay->add_child(colour_rect);

			colour_label = memnew(Label);
			colour_label->set_custom_minimum_size(Size2(40.0f, 40.0f));
			colour_label->set_align(Label::Align::ALIGN_CENTER);
			colour_label->set_valign(Label::VAlign::VALIGN_CENTER);
			overlay->add_child(colour_label);
		}
		colour_box->add_child(overlay);	
	
	
		Button *next_colour = memnew(Button);
		next_colour->set_text("->");
		next_colour->connect("pressed", this, "change_colour", make_binds(+1));
		next_colour->set_custom_minimum_size(Size2(40.0f, 40.0f));
		colour_box->add_child(next_colour);
	}
	add_child(colour_box);

	// Final setup
	set_colour(0);
	update_status();
	tool_select(TOOL_CUBE);
	update_palette_labels();
}
#endif // TOOLS_ENABLED