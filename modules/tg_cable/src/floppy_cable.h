#pragma once
#include <scene/3d/mesh_instance_3d.h>
#include <core/tg_util.h>
#include <vector>

struct CableParticle 
{
	bool is_free = true;
	Vector3 translation;
	Vector3 old_translation;
};

class FloppyCable3D : public MeshInstance3D {
	GDCLASS(FloppyCable3D, MeshInstance3D)

public:
	static void _bind_methods();
	void _notification(int p_what);

	void _init() {} // our initializer called by Godot
	void _ready();
	void _process(float delta);

	Vector3 get_position_on_cable(float alpha) const;

private:
	// Funcs
	void reset_cable();
	void solve_constraints();
	void verlet_integrate(float substep_time);

	void rebuild_mesh();

	int get_vert_index(int along_idx, int around_idx) const;

	// Exports
	DECLARE_PROPERTY(bool, is_start_attached, true);
	DECLARE_PROPERTY(Vector3, start_location, {});

	DECLARE_PROPERTY(bool, is_end_attached, false);
	DECLARE_PROPERTY(Vector3, end_location, {});

	DECLARE_PROPERTY(float, stiffness_coefficient, 1.0f);

	DECLARE_PROPERTY(float, cable_length, 1.0f);
	DECLARE_PROPERTY(float, cable_width, 0.2f);
	DECLARE_PROPERTY(int, cable_num_segments, 8);
	DECLARE_PROPERTY(int, cable_num_sides, 8);
	DECLARE_PROPERTY(bool, reverse_winding_order, false);

	DECLARE_PROPERTY(Ref<Material>, cable_material, {});

	// State	
	std::vector<CableParticle> particles;
	float time_remainder = 0.0f;
};
