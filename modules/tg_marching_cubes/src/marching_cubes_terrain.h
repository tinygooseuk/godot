#pragma once
#include <scene/3d/mesh_instance.h>
#include <core/tg_util.h>
#include <vector>

#include "marching_cubes_data.h"

class CollisionShape;

class MarchingCubesTerrain : public MeshInstance {
	GDCLASS(MarchingCubesTerrain, MeshInstance)

public:
	static void _bind_methods();
	void _notification(int p_what);

	void _init();
	void _ready();
	void _process(float delta);

	virtual String get_configuration_warning() const override;

	float get_value_at(const Vector3& p_position) const;
	void set_value_at(const Vector3& p_position, float p_value);

	void generate_mesh();

private:
	// Exports
	DECLARE_PROPERTY(float, mesh_scale, 1.0f);
	DECLARE_PROPERTY(bool, generate_collision, true);
	DECLARE_PROPERTY(bool, regenerate_mesh, false);

	DECLARE_PROPERTY(Ref<MarchingCubesData>, terrain_data, {});
	
	DECLARE_PROPERTY(Ref<Material>, tops_material, {});
	DECLARE_PROPERTY(Ref<Material>, sides_material, {});

	int coord_to_index(const Vector3& p_position) const;
	Vector3 index_to_coord(int p_index) const;

	CollisionShape* find_collision_sibling() const;

	void reallocate_memory();
	void fill_with_noise();
};
