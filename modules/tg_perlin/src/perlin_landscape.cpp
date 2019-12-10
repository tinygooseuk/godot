#include "perlin_landscape.h"

#include "stb_perlin.h"

void PerlinLandscape::_bind_methods() {
	// State
	IMPLEMENT_PROPERTY(PerlinLandscape, BOOL, needs_generation);

	// Setup
	IMPLEMENT_PROPERTY_TYPEHINT(PerlinLandscape, OBJECT, Material, landscape_material);
	IMPLEMENT_PROPERTY(PerlinLandscape, REAL, chunk_size);
	IMPLEMENT_PROPERTY(PerlinLandscape, REAL, perlin_seed);
	IMPLEMENT_PROPERTY(PerlinLandscape, REAL, perlin_height);
	IMPLEMENT_PROPERTY(PerlinLandscape, REAL, perlin_scale_factor);
	IMPLEMENT_PROPERTY(PerlinLandscape, REAL, perlin_tile_size);
	IMPLEMENT_PROPERTY(PerlinLandscape, REAL, top_cutoff);
	IMPLEMENT_PROPERTY(PerlinLandscape, REAL, bottom_cutoff);
}

void PerlinLandscape::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			_ready();
			set_process(true);
			break;
		case NOTIFICATION_PROCESS:
			_process(get_process_delta_time());
			break;
		default:
			break;
	}
}

void PerlinLandscape::_ready() {
}

void PerlinLandscape::_process(const float delta) {
	if (needs_generation) {
		needs_generation = false;
		rebuild_mesh();
	}
}

void PerlinLandscape::rebuild_mesh() {
	const uint32_t grid_resolution = uint32_t(chunk_size / perlin_tile_size);

	// The heightmap function
	auto height_func = [&grid_resolution, this](int32_t x, int32_t y) {
		float raw_perlin = stb_perlin_noise3(x * perlin_scale_factor, y * perlin_scale_factor, perlin_seed);

		return clamp(raw_perlin, bottom_cutoff, top_cutoff) * perlin_height;
	};

	PoolVector3Array vertices;
	vertices.resize(grid_resolution * grid_resolution * 6);
	auto v_writer = vertices.write();
	uint32_t v_index = 0;
	
	PoolVector3Array normals;
	normals.resize(grid_resolution * grid_resolution * 6);
	auto n_writer = normals.write();
	uint32_t n_index = 0;
	
	// build...
	Vector3 centre_offset = Vector3(chunk_size / 2.0f, 0.0f, chunk_size / 2.0f); // Offset by half-size so origin is in centre

	// Compile vertices tile-by-tile
	for (uint32_t x = 0; x < grid_resolution; ++x) {
		for (uint32_t y = 0; y < grid_resolution; ++y) {
			const Vector3 start_pos = Vector3(x * perlin_tile_size, 0.0f, y * perlin_tile_size) - centre_offset;

			// Work out corner positions
			const Vector3 tl = Vector3(start_pos.x, height_func(x, y), start_pos.z);
			const Vector3 bl = Vector3(start_pos.x, height_func(x, y + 1), start_pos.z + perlin_tile_size);
			const Vector3 br = Vector3(start_pos.x + perlin_tile_size, height_func(x + 1, y + 1), start_pos.z + perlin_tile_size);
			const Vector3 tr = Vector3(start_pos.x + perlin_tile_size, height_func(x + 1, y), start_pos.z);

			// Compute normals
			const Vector3 x_grad_bl = (br - bl);
			const Vector3 y_grad_bl = (tl - bl);

			const Vector3 normal_bl = y_grad_bl.cross(x_grad_bl).normalized();

			const Vector3 x_grad_tr = (tr - tl);
			const Vector3 y_grad_tr = (tr - br);

			const Vector3 normal_tr = y_grad_tr.cross(x_grad_tr).normalized();

			// Add vertices
			v_writer[v_index++] = bl;
			v_writer[v_index++] = tl;
			v_writer[v_index++] = br;
			v_writer[v_index++] = tl;
			v_writer[v_index++] = tr;
			v_writer[v_index++] = br;

			n_writer[n_index++] = normal_bl;
			n_writer[n_index++] = normal_bl;
			n_writer[n_index++] = normal_bl;
			n_writer[n_index++] = normal_tr;
			n_writer[n_index++] = normal_tr;
			n_writer[n_index++] = normal_tr;
		}
	}

	const Variant nil;
	
	Array arrays;
	arrays.resize(9);

	arrays[0] = vertices;
	arrays[1] = normals;
	arrays[2] = nil;
	arrays[3] = nil;
	arrays[4] = nil;
	arrays[5] = nil;
	arrays[6] = nil;
	arrays[7] = nil;
	arrays[8] = nil;
	
	ArrayMesh *mesh = memnew(ArrayMesh);
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	mesh->surface_set_material(0, landscape_material);
	set_mesh(mesh);
}
