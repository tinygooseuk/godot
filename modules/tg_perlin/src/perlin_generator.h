#pragma once
#include <core/object.h>

class PerlinGenerator : public Object
{
	GDCLASS(PerlinGenerator, Object) 

public:
	static void _bind_methods();

	// Generate Perlin noise at (x, y, z)
	float perlin_noise(const Vector3 &pos) const;
	float perlin_noise_ext(const Vector3 &pos, int x_wrap, int y_wrap, int z_wrap) const;
};
