#pragma once
#include <core/tg_util.h>
#include <scene/3d/mesh_instance_3d.h>

#include "marching_cubes_data.h"

class CollisionShape3D;

class MarchingCubesTerrain3D : public MeshInstance3D {
	GDCLASS(MarchingCubesTerrain3D, MeshInstance3D)

public:
	static void _bind_methods();
	void _notification(int p_what);

	void _init();
	void ready();
	void process(float delta);

	virtual String get_configuration_warning() const /*override*/;

	float get_value_at(const Vector3 &p_position) const;
	void set_value_at(const Vector3 &p_position, float p_value);

	int get_colour_at(const Vector3 &p_position) const;
	void set_colour_at(const Vector3 &p_position, int p_colour);

	// Tools
	void brush_cube(const Vector3 &centre, float radius, float power, bool additive = true);
	void brush_sphere(const Vector3 &centre, float radius, float power, bool additive = true);
	void paint_sphere(const Vector3 &centre, float radius, int colour);
	void flatten_cube(const Vector3 &centre, float radius, float power);
	void ruffle_cube(const Vector3 &centre, float radius, float power);

	bool are_grid_coordinates_valid(const Vector3 &p_coords) const;

	Vector3 get_grid_coordinates_from_world_position(Vector3 p_world_pos) const;
	Vector3 get_world_position_from_grid_coordinates(Vector3 p_coords) const;

	void generate_mesh();
	void generate_debug_mesh();

private:
	// Exports
	DECLARE_PROPERTY(float, mesh_scale, 1.0f);
	DECLARE_PROPERTY(bool, generate_collision, true);
#if TOOLS_ENABLED
	DECLARE_PROPERTY(bool, debug_mode, false);
	bool old_debug_mode = false;
#endif
	DECLARE_PROPERTY(bool, is_destructible, false);

	DECLARE_PROPERTY(Ref<MarchingCubesData>, terrain_data, {});

	DECLARE_PROPERTY(Ref<Material>, tops_material, {});
	DECLARE_PROPERTY(Ref<Material>, sides_material, {});

	int coord_to_index(const Vector3 &p_position) const;
	Vector3 index_to_coord(int p_index) const;

	CollisionShape3D *generate_collision_shape(Ref<Shape3D> p_shape);
	CollisionShape3D *find_collision_sibling() const;

	void clear_mesh();
	void reallocate_memory();
	void fill_with_noise();
	void invert_data_sign();

#if TOOLS_ENABLED
	friend class MarchingCubesEditor;
	friend class MarchingCubesEditorPlugin;
#endif
};
