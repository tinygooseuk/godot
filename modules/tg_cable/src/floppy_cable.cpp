#include "floppy_cable.h"
#include "core/engine.h"

namespace {

void solve_distance_constraint(CableParticle &particle1, CableParticle &particle2, const float desired_distance) {
	// Find current vector between particles
	const Vector3 delta = particle2.translation - particle1.translation;

	const float current_distance = delta.length();
	const float error_factor = (current_distance - desired_distance) / current_distance;

	// Only move free particles to satisfy constraints
	if (particle1.is_free && particle2.is_free) {
		particle1.translation += error_factor * 0.5f * delta;
		particle2.translation -= error_factor * 0.5f * delta;
	} else if (particle1.is_free) {
		particle1.translation += error_factor * delta;
	} else if (particle2.is_free) {
		particle2.translation -= error_factor * delta;
	}
}

const Vector3 VECTOR_X = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 VECTOR_Y = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 VECTOR_Z = Vector3(0.0f, 0.0f, 1.0f);
} // namespace

void FloppyCable3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_position_on_cable", "distance_along_cable"), &FloppyCable3D::get_position_on_cable);

	IMPLEMENT_PROPERTY(FloppyCable3D, BOOL, is_start_attached);

	IMPLEMENT_PROPERTY(FloppyCable3D, VECTOR3, start_location);

	IMPLEMENT_PROPERTY(FloppyCable3D, BOOL, is_end_attached);
	IMPLEMENT_PROPERTY(FloppyCable3D, VECTOR3, end_location);

	IMPLEMENT_PROPERTY(FloppyCable3D, FLOAT, stiffness_coefficient);

	IMPLEMENT_PROPERTY(FloppyCable3D, FLOAT, cable_length);
	IMPLEMENT_PROPERTY(FloppyCable3D, FLOAT, cable_width);
	IMPLEMENT_PROPERTY(FloppyCable3D, INT, cable_num_segments);
	IMPLEMENT_PROPERTY(FloppyCable3D, INT, cable_num_sides);
	IMPLEMENT_PROPERTY(FloppyCable3D, BOOL, reverse_winding_order);

	IMPLEMENT_PROPERTY_RESOURCE(FloppyCable3D, Material, cable_material);
}

void FloppyCable3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			_ready();
			set_process(!Engine::get_singleton()->is_editor_hint()); 
			break;
		case NOTIFICATION_PROCESS:
			if (!Engine::get_singleton()->is_editor_hint()) {
				_process(get_process_delta_time());
			}
			break;
		default:
			break;
	}
	//ODO: hookup!
}

void FloppyCable3D::_ready() {
	reset_cable();
}

void FloppyCable3D::_process(const float delta) {
	// Update end points
	CableParticle &start_particle = particles[0];
	CableParticle &end_particle = particles[cable_num_segments];

	// If start is attached - we drive its pos
	if (is_start_attached) {
		start_particle.translation = to_global(start_location);
		start_particle.old_translation = to_global(start_location);
		start_particle.is_free = false;
	} else {
		start_particle.is_free = true;
	}

	// If end is attached - we drive its pos
	if (is_end_attached) {
		end_particle.translation = to_global(end_location);
		end_particle.old_translation = to_global(end_location);
		end_particle.is_free = false;
	} else {
		end_particle.is_free = true;
	}

	// Perform simulation substeps
	time_remainder += delta;

	constexpr float SUBSTEP = 0.02f;
	while (time_remainder > SUBSTEP) {
		verlet_integrate(SUBSTEP);
		solve_constraints();
		time_remainder -= SUBSTEP;
	}

	// If start/end not attached, copy back the position
	if (!is_start_attached) {
		start_location = to_local(start_particle.translation);
	}
	if (!is_end_attached) {
		end_location = to_local(end_particle.translation);
	}

	rebuild_mesh();
}

Vector3 FloppyCable3D::get_position_on_cable(float alpha) const {
	const float particle_pos = alpha * static_cast<float>(particles.size());

	const int32_t particle_idx = static_cast<int32_t>(particle_pos);
	const float particle_alpha = particle_pos - static_cast<float>(particle_idx);

	const Vector3 particle1_pos = particles[particle_idx].translation;
	const Vector3 particle2_pos = particles[min((int)particles.size() - 1, particle_idx + 1)].translation;

	const Vector3 diff = particle2_pos - particle1_pos;
	return particle1_pos + diff * particle_alpha;
}

void FloppyCable3D::reset_cable() {
	const Vector3 delta = to_global(end_location) - to_global(start_location);

	particles.resize(cable_num_segments + 1);
	for (int particle_idx = 0; particle_idx < cable_num_segments + 1; particle_idx++) {
		const float alpha = float(particle_idx) / float(cable_num_segments + 1);
		const Vector3 initial_trans = to_global(start_location) + (alpha * delta);

		CableParticle particle;
		particle.translation = initial_trans;
		particle.old_translation = initial_trans;
		particle.is_free = true; // default to free, will be fixed if desired in TickComponent
		particles[particle_idx] = particle;
	}
}

void FloppyCable3D::solve_constraints() {
	const float segment_length = cable_length / float(cable_num_segments);

	// Solve distance constraint for each segment
	for (int i = 0; i < cable_num_segments; i++) {
		CableParticle &particle1 = particles[i];
		CableParticle &particle2 = particles[i + 1];

		// Solve for this pair of particles
		solve_distance_constraint(particle1, particle2, segment_length);
	}

	// If desired, solve stiffness constraints(distance constraints between every other particle)
	if (stiffness_coefficient > 1.0f) {
		for (int i = 0; i < cable_num_segments; i++) {
			CableParticle &particle1 = particles[i];
			CableParticle &particle2 = particles[i + 1];

			// Solve for this pair of particles
			solve_distance_constraint(particle1, particle2, stiffness_coefficient * segment_length);
		}
	}
}

void FloppyCable3D::verlet_integrate(const float substep_time) {
	const float gravity_amount = ProjectSettings::get_singleton()->get_setting("physics/3d/default_gravity");
	const Vector3 gravity = Vector3(0.0f, -gravity_amount, 0.0f);

	const int num_particles = cable_num_segments + 1;
	const float substep_time_sqr = substep_time * substep_time;

	for (int particle_idx = 0; particle_idx < num_particles; particle_idx++) {
		CableParticle &particle = particles[particle_idx];

		if (particle.is_free) {
			const Vector3 velocity = particle.translation - particle.old_translation;

			// Update position
			const Vector3 new_translation = particle.translation + velocity + (substep_time_sqr * gravity);

			particle.old_translation = particle.translation;
			particle.translation = new_translation;
		}
	}
}

void FloppyCable3D::rebuild_mesh() {
	const int num_points = cable_num_segments + 1;

	// We double up the first and last vert of the ring, because the UVs are different
	const int num_ring_verts = cable_num_sides + 1;

	PackedVector3Array vertices;
	vertices.resize(num_ring_verts * num_points);
	PackedVector2Array tex_coords;
	tex_coords.resize(num_ring_verts * num_points);
	PackedVector3Array normals;
	normals.resize(num_ring_verts * num_points);

	uint32_t vertex_idx = 0;
	uint32_t texcoord_idx = 0;
	uint32_t normal_idx = 0;

	const float winding_inversion_factor = reverse_winding_order ? -1.0f : +1.0f;

	// For each point along spline...
	for (int point_idx = 0; point_idx < num_points; point_idx++) {
		const float along_frac = float(point_idx) / float(num_points); // Distance along cable

		// Find direction of cable at this point, by averaging previous and next points
		const int last_idx = point_idx - 1;
		const int next_idx = point_idx + 1;

		// Create basis
		CableParticle &next_particle = (point_idx == num_points - 1) ? particles[point_idx] : particles[next_idx];
		CableParticle &curr_particle = (point_idx == num_points - 1) ? particles[last_idx] : particles[point_idx];

		const Vector3 along_dir = (next_particle.translation - curr_particle.translation).normalized();
		Vector3 up_dir = VECTOR_Y;

		//TODO: twisty
		// Along dir is mostly on y/UP axis - must use different up
		if (abs(along_dir.y) > max(abs(along_dir.x), abs(along_dir.z))) {
			if (abs(along_dir.x) > abs(along_dir.z)) {
				up_dir = along_dir.cross(-VECTOR_Z);
			} else {
				up_dir = along_dir.cross(VECTOR_X);
			}
		}

		Vector3	right_dir = along_dir.cross(up_dir).normalized();
		
		// Generate a ring of verts
		for (int vert_idx = 0; vert_idx < num_ring_verts; vert_idx++) {
			const float around_frac = fmod(float(vert_idx) / float(cable_num_sides), 1.0f);

			// Find angle around the ring
			const float rad_angle = 2.0f * Math_PI * around_frac;

			// Find direction from center of cable to this vertex
			const Vector3 out_dir = (cosf(rad_angle) * up_dir) + (winding_inversion_factor * sinf(rad_angle) * right_dir);

			vertices.set(vertex_idx++, to_local(curr_particle.translation + (out_dir * 0.5f * cable_width)));
			tex_coords.set(texcoord_idx++, Vector2(along_frac, around_frac)); // unreal had : FVector2D(AlongFrac * TileMaterial, AroundFrac);
			normals.set(normal_idx++, out_dir);
		}
	}

	// Build triangles
	PackedInt32Array indices;
	indices.resize(2 * 3 * cable_num_segments * cable_num_sides);

	uint32_t index_idx = 0;
	
	for (int seg_idx = 0; seg_idx < cable_num_segments; seg_idx++) {
		for (int side_idx = 0; side_idx < cable_num_sides; side_idx++) {
			const int tl = get_vert_index(seg_idx, side_idx);
			const int bl = get_vert_index(seg_idx, side_idx + 1);
			const int tr = get_vert_index(seg_idx + 1, side_idx);
			const int br = get_vert_index(seg_idx + 1, side_idx + 1);

			indices.set(index_idx++, tl);
			indices.set(index_idx++, bl);
			indices.set(index_idx++, tr);

			indices.set(index_idx++, bl);
			indices.set(index_idx++, br);
			indices.set(index_idx++, tr);
		}
	}

	const Variant nil;

	Array arrays;
	arrays.resize(9);

	arrays[0] = vertices;
	arrays[1] = normals;
	arrays[2] = nil;
	arrays[3] = nil;
	arrays[4] = tex_coords;
	arrays[5] = nil;
	arrays[6] = nil;
	arrays[7] = nil;
	arrays[8] = indices;

	ArrayMesh *mesh = memnew(ArrayMesh);
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	mesh->surface_set_material(0, cable_material);
	set_mesh(mesh);
}

int FloppyCable3D::get_vert_index(const int along_idx, const int around_idx) const {
	return (along_idx * (cable_num_sides + 1)) + around_idx;
}
