#pragma once
#include <scene/3d/mesh_instance.h>

#include <core/tg_util.h>			

class PerlinLandscape : public MeshInstance
{
	GDCLASS(PerlinLandscape, MeshInstance)

public:
	void _notification(int p_what);
	static void _bind_methods();

	void _ready();
	void _process(float delta);
	
private:
	// Funcs
	void rebuild_mesh();

	// State
	DEFINE_PROPERTY(bool, needs_generation, true);
	
	// Setup
	DEFINE_PROPERTY(Ref<Material>, landscape_material, {});

	DEFINE_PROPERTY(float, chunk_size, 3.0f);

	DEFINE_PROPERTY(float, perlin_seed, 10.0f);

	DEFINE_PROPERTY(float, perlin_height, 0.14f);
	DEFINE_PROPERTY(float, perlin_scale_factor, 0.4f);
	DEFINE_PROPERTY(float, perlin_tile_size, 0.25f);
	
	DEFINE_PROPERTY(float, top_cutoff, 1.0f);
	DEFINE_PROPERTY(float, bottom_cutoff, -1.0f);
};
