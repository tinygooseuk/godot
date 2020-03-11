#pragma once
#include <core/resource.h>
#include <core/tg_util.h>

class MarchingCubesData : public Resource {
	GDCLASS(MarchingCubesData, Resource)

public:
	static void _bind_methods() {
		IMPLEMENT_PROPERTY(MarchingCubesData, INT, width);
		IMPLEMENT_PROPERTY(MarchingCubesData, INT, height);
		IMPLEMENT_PROPERTY(MarchingCubesData, INT, depth);

		IMPLEMENT_PROPERTY(MarchingCubesData, INT, random_seed);

		IMPLEMENT_PROPERTY(MarchingCubesData, POOL_REAL_ARRAY, data);

		IMPLEMENT_PROPERTY(MarchingCubesData, BOOL, use_colour);
		IMPLEMENT_PROPERTY(MarchingCubesData, POOL_BYTE_ARRAY, colour_data);

		IMPLEMENT_PROPERTY(MarchingCubesData, POOL_COLOR_ARRAY, colour_palette);
	}

	DECLARE_PUBLIC_PROPERTY(int, width, 8);
	DECLARE_PUBLIC_PROPERTY(int, height, 8);
	DECLARE_PUBLIC_PROPERTY(int, depth, 8);

	DECLARE_PUBLIC_PROPERTY(int, random_seed, 8);

	DECLARE_PUBLIC_PROPERTY(PoolRealArray, data, {});

	DECLARE_PUBLIC_PROPERTY(bool, use_colour, false);
	DECLARE_PUBLIC_PROPERTY(PoolByteArray, colour_data, {});
	DECLARE_PUBLIC_PROPERTY(PoolColorArray, colour_palette, {});
};