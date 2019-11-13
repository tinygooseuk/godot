#pragma once
#include <scene/3d/mesh_instance.h>
#include <core/tg_util.h>
#include <vector>

struct CableParticle 
{
	bool is_free = true;
	Vector3 translation;
	Vector3 old_translation;
};

class FloppyCable : public MeshInstance {
	GDCLASS(FloppyCable, MeshInstance)

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
	DEFINE_PROPERTY(bool, is_start_attached, true);
	DEFINE_PROPERTY(Vector3, start_location, {});

	DEFINE_PROPERTY(bool, is_end_attached, false);
	DEFINE_PROPERTY(Vector3, end_location, {});

	DEFINE_PROPERTY(bool, use_stiffness, false);

	DEFINE_PROPERTY(float, cable_length, 1.0f);
	DEFINE_PROPERTY(float, cable_width, 0.2f);
	DEFINE_PROPERTY(int, cable_num_segments, 8);
	DEFINE_PROPERTY(int, cable_num_sides, 8);

	DEFINE_PROPERTY(Ref<Material>, cable_material, {});

	// State	
	std::vector<CableParticle> particles;
	float time_remainder = 0.0f;
};
