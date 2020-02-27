#include "marching_cubes_terrain.h"
#include "core/engine.h"
#include "modules/opensimplex/open_simplex_noise.h"
#include "scene/resources/concave_polygon_shape.h"
#include "scene/resources/surface_tool.h"
#include "scene/3d/collision_shape.h"

#include "marching_cubes_algorithm.h"

#define MC_REPORT_ERRORS 0

#if MC_REPORT_ERRORS
#define MC_ERR_FAIL_COND(cond)			ERR_FAIL_COND(cond)
#define MC_ERR_FAIL_COND_V(cond, val) 	ERR_FAIL_COND_V(cond, val)
#else
#define MC_ERR_FAIL_COND(cond)			do { if (cond) return; } while (false);
#define MC_ERR_FAIL_COND_V(cond, val) 	do { if (cond) return val; } while (false);
#endif

void MarchingCubesTerrain::_bind_methods() {
	IMPLEMENT_PROPERTY_RESOURCE(MarchingCubesTerrain, MarchingCubesData, terrain_data);
	IMPLEMENT_PROPERTY_RESOURCE(MarchingCubesTerrain, Material, tops_material);
	IMPLEMENT_PROPERTY_RESOURCE(MarchingCubesTerrain, Material, sides_material);
	
	IMPLEMENT_PROPERTY(MarchingCubesTerrain, REAL, mesh_scale);
	IMPLEMENT_PROPERTY(MarchingCubesTerrain, BOOL, generate_collision);	
#if TOOLS_ENABLED
	IMPLEMENT_PROPERTY(MarchingCubesTerrain, BOOL, debug_mode);	
#endif
	IMPLEMENT_PROPERTY(MarchingCubesTerrain, BOOL, is_destructible);

	ClassDB::bind_method(D_METHOD("get_value_at", "position"), &MarchingCubesTerrain::get_value_at);
	ClassDB::bind_method(D_METHOD("set_value_at", "position", "value"), &MarchingCubesTerrain::set_value_at);
	
	ClassDB::bind_method(D_METHOD("brush_cube", "centre", "radius", "power", "additive"), &MarchingCubesTerrain::brush_cube);
	ClassDB::bind_method(D_METHOD("brush_sphere", "centre", "radius", "power", "additive"), &MarchingCubesTerrain::brush_sphere);
	ClassDB::bind_method(D_METHOD("flatten_cube", "centre", "radius", "power"), &MarchingCubesTerrain::flatten_cube);
	ClassDB::bind_method(D_METHOD("ruffle_cube", "centre", "radius", "power"), &MarchingCubesTerrain::ruffle_cube);

	ClassDB::bind_method(D_METHOD("are_grid_coordinates_valid", "grid_position"), &MarchingCubesTerrain::are_grid_coordinates_valid);
	ClassDB::bind_method(D_METHOD("get_grid_coordinates_from_world_position", "world_position"), &MarchingCubesTerrain::get_grid_coordinates_from_world_position);
	ClassDB::bind_method(D_METHOD("get_world_position_from_grid_coordinates", "grid_position"), &MarchingCubesTerrain::get_world_position_from_grid_coordinates);
	
	ClassDB::bind_method(D_METHOD("generate_mesh"), &MarchingCubesTerrain::generate_mesh);
	ClassDB::bind_method(D_METHOD("generate_collision_shape"), &MarchingCubesTerrain::generate_collision_shape);
}

void MarchingCubesTerrain::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			ready();
			set_process(Engine::get_singleton()->is_editor_hint()); 
			break;

		case NOTIFICATION_PROCESS:
			process(get_process_delta_time());
		default:
			break;
	}
}

void MarchingCubesTerrain::_init() {
	if (terrain_data.is_null()) {
		//TODO: memory leak?
		terrain_data = Ref<MarchingCubesData>(memnew(MarchingCubesData));
	}
}

void MarchingCubesTerrain::ready() {
	
}

void MarchingCubesTerrain::process(const float delta) {
	if (old_debug_mode != debug_mode) {
		old_debug_mode = debug_mode;
		generate_mesh();
	}
}

String MarchingCubesTerrain::get_configuration_warning() const {
	if (terrain_data.is_null()) {
		return "No terrain data specified!";
	}
	return "";
}

float MarchingCubesTerrain::get_value_at(const Vector3& p_position) const {
	const int index = coord_to_index(p_position);
	if (index == -1) {
		return 0.0f;
	}

	return terrain_data->data.read()[index];
}
void MarchingCubesTerrain::set_value_at(const Vector3& p_position, float p_value) {
	const int index = coord_to_index(p_position);

	if (index != -1) {
		terrain_data->data.write()[index] = clamp(p_value, -1.0f, +1.0f);
	}
}


void MarchingCubesTerrain::brush_cube(const Vector3 &centre, float radius, float power, bool additive) {
	for (float x = -radius; x <= +radius; x += 1.0f) {
		for (float y = -radius; y <= +radius; y += 1.0f) {
			for (float z = -radius; z <= +radius; z += 1.0f) {
				const Vector3 coord = get_grid_coordinates_from_world_position(centre + Vector3(x, y, z));

				if (are_grid_coordinates_valid(coord)) {
					const float currentValue = get_value_at(coord);
					float targetValue = power;
					if (additive) {
						targetValue += currentValue;
					}

					set_value_at(coord, targetValue);
				}
			}
		}
	}
}

void MarchingCubesTerrain::brush_sphere(const Vector3 &centre, float radius, float power, bool additive) {
	for (float x = -radius; x <= +radius; x += 1.0f) {
		for (float y = -radius; y <= +radius; y += 1.0f) {
			for (float z = -radius; z <= +radius; z += 1.0f) {
				const Vector3 offset = Vector3(x, y, z);
				Vector3 coord = get_grid_coordinates_from_world_position(centre + offset);

				if (are_grid_coordinates_valid(coord)) {
					const float current_value = get_value_at(coord);
					float target_value = power;
					if (additive) {
						target_value += current_value;
					}
					
					const float alpha = clamp(offset.length() / radius, 0.0f, 1.0f);
					set_value_at(coord, Math::lerp(current_value, target_value, alpha));
				}
			}
		}
	}
}
 
void MarchingCubesTerrain::flatten_cube(const Vector3 &centre, float radius, float power) {
	const int half_range = (int)Math::ceil(radius / mesh_scale);
	float mean_value = 0.0f;

	for (int x = -half_range; x <= +half_range; x++) {
		for (int y = -half_range; y <= +half_range; y++) {
			for (int z = -half_range; z <= +half_range; z++) {
				const Vector3 offset = Vector3(x, y, z) * mesh_scale;
				const Vector3 coord = get_grid_coordinates_from_world_position(centre + offset);

				if (are_grid_coordinates_valid(coord)) {
					mean_value += get_value_at(coord);
				}				
			}
		}
	}

	mean_value /= (half_range * 3.0f * 2.0f); // full range, one 3 axes

	for (int x = -half_range; x <= +half_range; x++) {
		for (int y = -half_range; y <= +half_range; y++) {
			for (int z = -half_range; z <= +half_range; z++) {
				const Vector3 offset = Vector3(x, y, z) * mesh_scale;
				const Vector3 coord = get_grid_coordinates_from_world_position(centre + offset);

				if (are_grid_coordinates_valid(coord)) {
					const float current_value = get_value_at(coord);					
					set_value_at(coord, Math::lerp(current_value, mean_value, power));
				}
			}
		}
	}
}

void MarchingCubesTerrain::ruffle_cube(const Vector3 &centre, float radius, float power) {
	const int half_range = (int)Math::ceil(radius / mesh_scale);

	for (int x = -half_range; x <= +half_range; x++) {
		for (int y = -half_range; y <= +half_range; y++) {
			for (int z = -half_range; z <= +half_range; z++) {
				Vector3 offset = Vector3(x, y, z) * mesh_scale;
				Vector3 coord = get_grid_coordinates_from_world_position(centre + offset);

				if (are_grid_coordinates_valid(coord)) {
					float current_value = get_value_at(coord);
					float target_value = current_value + Math::random(-power, +power);

					float alpha = offset.length() / radius;
					set_value_at(coord, Math::lerp(current_value, target_value, alpha));
				}
			}
		}
	}
}

bool MarchingCubesTerrain::are_grid_coordinates_valid(const Vector3& p_coords) const {
	MC_ERR_FAIL_COND_V(p_coords.x < 0.0f || p_coords.x > terrain_data->width, false);
	MC_ERR_FAIL_COND_V(p_coords.y < 0.0f || p_coords.y > terrain_data->height, false);
	MC_ERR_FAIL_COND_V(p_coords.z < 0.0f || p_coords.z > terrain_data->depth, false);

	return true;
}

Vector3 MarchingCubesTerrain::get_grid_coordinates_from_world_position(Vector3 p_world_pos) const {
	p_world_pos -= get_global_transform().get_origin();
	p_world_pos /= mesh_scale;

	MC_ERR_FAIL_COND_V(p_world_pos.x < 0.0f || p_world_pos.x > terrain_data->width, p_world_pos);
	MC_ERR_FAIL_COND_V(p_world_pos.y < 0.0f || p_world_pos.y > terrain_data->height, p_world_pos);
	MC_ERR_FAIL_COND_V(p_world_pos.z < 0.0f || p_world_pos.z > terrain_data->depth, p_world_pos);

	return p_world_pos.round();
}
Vector3 MarchingCubesTerrain::get_world_position_from_grid_coordinates(Vector3 p_coords) const {
	MC_ERR_FAIL_COND_V(p_coords.x < 0.0f || p_coords.x > terrain_data->width, p_coords);
	MC_ERR_FAIL_COND_V(p_coords.y < 0.0f || p_coords.y > terrain_data->height, p_coords);
	MC_ERR_FAIL_COND_V(p_coords.z < 0.0f || p_coords.z > terrain_data->depth, p_coords);

	p_coords *= mesh_scale;
	p_coords += get_global_transform().get_origin();

	return p_coords;
}

void MarchingCubesTerrain::generate_mesh() {
	MC_ERR_FAIL_COND(terrain_data.is_null() || terrain_data->data.empty());

	if (debug_mode) {
		return generate_debug_mesh();
	}

	auto data_read = terrain_data->data.read();
	
	Ref<ArrayMesh> new_mesh = get_mesh();
	
	if (new_mesh.is_null()) {
		new_mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
		set_mesh(new_mesh);
	} else {
		while (new_mesh->get_surface_count() > 0) {
			new_mesh->surface_remove(0);
		}
	}

	SurfaceTool sides;
	SurfaceTool tops;

	sides.begin(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);
	tops.begin(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);
	{
		for (int x = 0; x < terrain_data->width; x++) {
			for (int y = 0; y < terrain_data->height; y++) {
				for (int z = 0; z < terrain_data->depth; z++) {
					MarchingCubes::GridCell grid_cell;
					grid_cell.position[0] = Vector3((float)(x + 0), (float)(y + 0), (float)(z + 0));
					grid_cell.position[1] = Vector3((float)(x + 1), (float)(y + 0), (float)(z + 0));
					grid_cell.position[2] = Vector3((float)(x + 1), (float)(y + 0), (float)(z + 1));
					grid_cell.position[3] = Vector3((float)(x + 0), (float)(y + 0), (float)(z + 1));
					grid_cell.position[4] = Vector3((float)(x + 0), (float)(y + 1), (float)(z + 0));
					grid_cell.position[5] = Vector3((float)(x + 1), (float)(y + 1), (float)(z + 0));
					grid_cell.position[6] = Vector3((float)(x + 1), (float)(y + 1), (float)(z + 1));
					grid_cell.position[7] = Vector3((float)(x + 0), (float)(y + 1), (float)(z + 1));
					
					for (int i = 0; i < 8; i++) {
						int index = coord_to_index(grid_cell.position[i]);
						if (index == -1) {
							grid_cell.value[i] = 0.0f;
						} else {						
							grid_cell.value[i] = data_read[index];
						}
					}

					MarchingCubes::Face faces[8];
					Vector3 vertices[16]; 
					int vert_count = 0;
					int face_count = MarchingCubes::polygonise(grid_cell, &faces[0], vert_count, &vertices[0]);
					
					for (int face_idx = 0; face_idx < face_count; face_idx++) {
						static const Vector3 VECTOR_UP = Vector3(0.0f, 1.0f, 0.0f);

						Vector3 a = vertices[faces[face_idx].indices[0]] * mesh_scale;
						Vector3 b = vertices[faces[face_idx].indices[2]] * mesh_scale;
						Vector3 c = vertices[faces[face_idx].indices[1]] * mesh_scale;

						Vector3 n = (b-a).cross(c-b).normalized();
						SurfaceTool& append_to = (n.dot(VECTOR_UP) > 0.55f) ? tops : sides;

						// Swap indices because GL is weird :)
						append_to.add_normal(-n);
						append_to.add_vertex(a);
						append_to.add_vertex(b);
						append_to.add_vertex(c);
					}
				}	
			}	
		}
	}

	// Commit surfaces to mesh (backwards!)
	sides.commit(new_mesh);
	tops.commit(new_mesh);

	// Set materials (forwards!)
	new_mesh->surface_set_material(0, tops_material);
	new_mesh->surface_set_material(1, sides_material);
	
	// Set collision
	if (generate_collision && new_mesh.is_valid()) {
		Ref<Shape> shape = new_mesh->create_trimesh_shape();
		
		if (!shape.is_null()) {
			CollisionShape* old_coll_shape = find_collision_sibling();
			if (old_coll_shape) {
				// Tweak old collision
				old_coll_shape->set_shape(shape);
			} else {
				// Generate new collision shape node AFTER setup
				call_deferred("generate_collision_shape", shape);
			}
		}
	}
}

void MarchingCubesTerrain::generate_debug_mesh() {
	MC_ERR_FAIL_COND(terrain_data.is_null() || terrain_data->data.empty());

	auto data_read = terrain_data->data.read();
	
	Ref<ArrayMesh> new_mesh = get_mesh();
	
	if (new_mesh.is_null()) {
		new_mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
		set_mesh(new_mesh);
	} else {
		while (new_mesh->get_surface_count() > 0) {
			new_mesh->surface_remove(0);
		}
	}

	SurfaceTool debug;
	debug.begin(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);
	{
		for (int x = 0; x < terrain_data->width; x++) {
			for (int y = 0; y < terrain_data->height; y++) {
				for (int z = 0; z < terrain_data->depth; z++) {
					const Vector3 coord = Vector3((float)x, (float)y, (float)z);
					const int index = coord_to_index(coord);
					const float value = data_read[index];
				
					auto add_cube = [&debug](const Vector3 &centre, float half_extents, const Color &color){
						const Vector3 FBL = centre + Vector3(-half_extents, -half_extents, +half_extents);
						const Vector3 FBR = centre + Vector3(+half_extents, -half_extents, +half_extents);
						const Vector3 FTL = centre + Vector3(-half_extents, +half_extents, +half_extents);
						const Vector3 FTR = centre + Vector3(+half_extents, +half_extents, +half_extents);
						const Vector3 RBL = centre + Vector3(-half_extents, -half_extents, -half_extents);
						const Vector3 RBR = centre + Vector3(+half_extents, -half_extents, -half_extents);
						const Vector3 RTL = centre + Vector3(-half_extents, +half_extents, -half_extents);
						const Vector3 RTR = centre + Vector3(+half_extents, +half_extents, -half_extents);

						debug.add_color(color);

						// BACK
						debug.add_vertex(RBL); debug.add_vertex(RBR); debug.add_vertex(RTR); 
						debug.add_vertex(RBL); debug.add_vertex(RTR); debug.add_vertex(RTL); 

						// FRONT
						debug.add_vertex(FBR); debug.add_vertex(FBL); debug.add_vertex(FTL);
						debug.add_vertex(FBR); debug.add_vertex(FTL); debug.add_vertex(FTR);

						// LEFT
						debug.add_vertex(FBL); debug.add_vertex(RBL); debug.add_vertex(RTL);
						debug.add_vertex(FBL); debug.add_vertex(RTL); debug.add_vertex(FTL);

						// RIGHT
						debug.add_vertex(RBR); debug.add_vertex(FBR); debug.add_vertex(FTR);
						debug.add_vertex(RBR); debug.add_vertex(FTR); debug.add_vertex(RTR);

						// BOTTOM
						debug.add_vertex(FBL); debug.add_vertex(FBR); debug.add_vertex(RBR);
						debug.add_vertex(FBL); debug.add_vertex(RBR); debug.add_vertex(RBL);

						// TOP
						debug.add_vertex(RTL); debug.add_vertex(RTR); debug.add_vertex(FTR);
						debug.add_vertex(RTL); debug.add_vertex(FTR); debug.add_vertex(FTL);
					};

					add_cube(coord * mesh_scale, Math::absf(value) * 0.5f, value > 0.0f ? Color(0.0f, 1.0f, 0.0f) : Color(1.0f, 0.0f, 0.0f));
				}	
			}	
		}
	}

	// Commit surfaces to mesh (backwards!)
	debug.commit(new_mesh);

	// Set materials (forwards!)
	SpatialMaterial *spat_mat = memnew(SpatialMaterial);
	spat_mat->set_flag(SpatialMaterial::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	new_mesh->surface_set_material(0, spat_mat);
}

int MarchingCubesTerrain::coord_to_index(const Vector3& p_position) const {
	if (p_position.x >= terrain_data->width || p_position.y >= terrain_data->height || p_position.z >= terrain_data->depth ||
		p_position.x < 0 || p_position.y < 0 || p_position.z < 0) {
		return -1;
	}

	return p_position.z * (terrain_data->width * terrain_data->height) +
			p_position.y * (terrain_data->width) +
			p_position.x;
}

Vector3 MarchingCubesTerrain::index_to_coord(int p_index) const {
	Vector3 out;

	out.z = floorf((float)p_index / (terrain_data->width * terrain_data->height));
	p_index -= out.z * (terrain_data->width * terrain_data->height);

	out.y = floorf((float)p_index / terrain_data->width);
	p_index -= out.y * terrain_data->width;

	out.x = p_index;
	return out;
}

CollisionShape* MarchingCubesTerrain::generate_collision_shape(Ref<Shape> p_shape) {
	CollisionShape *cshape = memnew(CollisionShape);
	cshape->set_shape(p_shape);
	cshape->set_name((String)get_name() + "_Collision");

	get_parent()->add_child(cshape);
	cshape->set_owner(get_parent()->get_owner());

	return cshape;
}

CollisionShape* MarchingCubesTerrain::find_collision_sibling() const {
	Node* parent_node = get_parent();
	for (int i = 0; i < parent_node->get_child_count(); i++) {
		Node* sibling_node = parent_node->get_child(i);
		if (sibling_node->get_class() == "CollisionShape") {
			return (CollisionShape*)sibling_node;
		}
	}

	return nullptr;
}

void MarchingCubesTerrain::clear_mesh() {
	auto data_write = terrain_data->data.write();
	for (int i = 0; i < terrain_data->data.size(); i++)	{
		data_write[i] = 0.0f;
	}
}

void MarchingCubesTerrain::reallocate_memory() {
	MC_ERR_FAIL_COND(terrain_data.is_null());

	int size = terrain_data->width * terrain_data->height * terrain_data->depth;

	if (terrain_data->data.size() == size) {
		return;
	}
	
	bool justShrink = (size < terrain_data->data.size());
	
	terrain_data->data.resize(size);
	
	if (justShrink) {
		return;
	}

	// Clear out
	clear_mesh();
}

void MarchingCubesTerrain::fill_with_noise() {
	MC_ERR_FAIL_COND(terrain_data.is_null());

	const int size = terrain_data->width * terrain_data->height * terrain_data->depth;
	terrain_data->data.resize(size);
	
	auto data_write = terrain_data->data.write(); 

	OpenSimplexNoise noiser;
	noiser.set_seed(terrain_data->random_seed);
	noiser.set_octaves(4);
	noiser.set_period(20.0f);
	noiser.set_persistence(0.8f);

	for (int i = 0; i < size; i++) {
		Vector3 coord = index_to_coord(i);

		data_write[i] = noiser.get_noise_3dv(coord) + 0.2f; // bias the noise a little
	}
}

void MarchingCubesTerrain::invert_data_sign() {
	MC_ERR_FAIL_COND(terrain_data.is_null());

	const int size = terrain_data->width * terrain_data->height * terrain_data->depth;
	auto data_write = terrain_data->data.write(); 

	for (int i = 0; i < size; i++) {
		data_write[i] = -data_write[i];
	}
}