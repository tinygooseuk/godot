#include "marching_cubes_terrain.h"
#include "core/engine.h"
#include "modules/opensimplex/open_simplex_noise.h"
#include "scene/resources/surface_tool.h"

#include "marching_cubes_algorithm.h"

void MarchingCubesTerrain::_bind_methods() {
	//ClassDB::bind_method(D_METHOD("get_position_on_cable", "distance_along_cable"), &MarchingCubesTerrain::get_position_on_cable);
	//IMPLEMENT_PROPERTY(MarchingCubesTerrain, BOOL, is_start_attached);
	IMPLEMENT_PROPERTY_TYPEHINT(MarchingCubesTerrain, OBJECT, MarchingCubesData, terrain_data);
	IMPLEMENT_PROPERTY(MarchingCubesTerrain, REAL, mesh_scale);

}

void MarchingCubesTerrain::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			_ready();
			set_process(!Engine::get_singleton()->is_editor_hint()); 
			break;
		case NOTIFICATION_PROCESS:
			if (!Engine::get_singleton()->is_editor_hint())
				_process(get_process_delta_time());
			break;
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

void MarchingCubesTerrain::_ready() {
	reallocate_memory();
	fill_with_noise(); //TODO: no!
	set_debug_mesh(); //TODO: no!
	if (!Engine::get_singleton()->is_editor_hint()) {
		
	}
}

void MarchingCubesTerrain::_process(const float delta) {
	
}

String MarchingCubesTerrain::get_configuration_warning() const {
	if (terrain_data.is_null()) {
		return "No terrain data specified!";
	}
	return "";
}

int MarchingCubesTerrain::coord_to_index(const Vector3& p_position) const {
	return 	p_position.z * (terrain_data->width * terrain_data->height) +
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


bool MarchingCubesTerrain::get_cube_at(const Vector3& p_position) const {
	return false;
}
void MarchingCubesTerrain::set_cube_at(const Vector3& p_position, bool p_state) {

}

void MarchingCubesTerrain::reallocate_memory() {
	ERR_FAIL_COND(terrain_data.is_null());

	int size = terrain_data->width * terrain_data->height * terrain_data->depth;
	terrain_data->data.resize(size);
	
	auto data_write = terrain_data->data.write(); 

	for (int i = 0; i < size; i++) {
		data_write[i] = 0.0f; 
	}
}

void MarchingCubesTerrain::fill_with_noise() {
	ERR_FAIL_COND(terrain_data.is_null());

	int size = terrain_data->width * terrain_data->height * terrain_data->depth;
	terrain_data->data.resize(size);
	
	auto data_write = terrain_data->data.write(); 

	OpenSimplexNoise noiser;
	noiser.set_seed(terrain_data->random_seed);
	noiser.set_octaves(4);
	noiser.set_period(20.0f);
	noiser.set_persistence(0.8f);

	for (int i = 0; i < size; i++) {
		Vector3 coord = index_to_coord(i);

		data_write[i] = noiser.get_noise_3dv(coord);
	}
}

void MarchingCubesTerrain::set_debug_mesh() {
	ERR_FAIL_COND(terrain_data.is_null() || terrain_data->data.empty());

	auto data_read = terrain_data->data.read();
	
	/*Ref<ArrayMesh> new_mesh = memnew(ArrayMesh);

	SurfaceTool sides;
	SurfaceTool tops;*/

	SurfaceTool st;
	st.begin(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);
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
					
					for (int i = 0; i < 8; i++)
					{
						grid_cell.value[i] = data_read[coord_to_index(grid_cell.position[i])];
					}

					MarchingCubes::Face faces[8];
					Vector3 vertices[16]; 
					int vert_count = 0;
					int face_count = MarchingCubes::polygonise(grid_cell, &faces[0], vert_count, &vertices[0]);
					
					for (int face_idx = 0; face_idx < face_count; face_idx++)
					{
						Vector3 a = vertices[faces[face_idx].indices[0]] * mesh_scale;
						Vector3 b = vertices[faces[face_idx].indices[2]] * mesh_scale;
						Vector3 c = vertices[faces[face_idx].indices[1]] * mesh_scale;

						// Swap indices because GL is weird :)
						st.add_vertex(a);
						st.add_vertex(b);
						st.add_vertex(c);
					}
				}	
			}	
		}
	}
	st.generate_normals();
	st.generate_tangents();

	set_mesh(st.commit());
}