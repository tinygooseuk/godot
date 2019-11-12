#include "perlin_generator.h"

#include "stb_perlin.h"

void PerlinGenerator::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("perlin_noise", "pos"), &PerlinGenerator::perlin_noise);
	ClassDB::bind_method(D_METHOD("perlin_noise_ext", "pos", "x_wrap", "y_wrap", "z_wrap"), &PerlinGenerator::perlin_noise_ext);
}

float PerlinGenerator::perlin_noise(const Vector3& pos) const
{
	return stb_perlin_noise3(pos.x, pos.y, pos.z, 0, 0, 0);
}

float PerlinGenerator::perlin_noise_ext(const Vector3 &pos, const int x_wrap, const int y_wrap, const int z_wrap) const {
	return stb_perlin_noise3(pos.x, pos.y, pos.z, x_wrap, y_wrap, z_wrap);
}
