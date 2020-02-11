#pragma once
#include <scene/3d/mesh_instance.h>
#include <core/tg_util.h>
#include <vector>

#include "marching_cubes_data.h"

class MarchingCubesTerrain : public MeshInstance {
	GDCLASS(MarchingCubesTerrain, MeshInstance)

public:
	static void _bind_methods();
	void _notification(int p_what);

	void _init();
	void _ready();
	void _process(float delta);

	virtual String get_configuration_warning() const override;

	bool get_cube_at(const Vector3& p_position) const;
	void set_cube_at(const Vector3& p_position, bool p_state);

	void reallocate_memory();

private:
	// Exports
	DECLARE_PROPERTY(Ref<MarchingCubesData>, terrain_data, {});
	
	int coord_to_index(const Vector3& p_position) const;
	Vector3 index_to_coord(int p_index) const;


	void fill_with_noise();
	void set_debug_mesh();
};
