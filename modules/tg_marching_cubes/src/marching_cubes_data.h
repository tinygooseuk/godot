#pragma once
#include <core/tg_util.h>
#include <core/resource.h>

class MarchingCubesData : public Resource {
	GDCLASS(MarchingCubesData, Resource)

public:
	static void _bind_methods()
    {
        IMPLEMENT_PROPERTY(MarchingCubesData, INT, width);
        IMPLEMENT_PROPERTY(MarchingCubesData, INT, height);
        IMPLEMENT_PROPERTY(MarchingCubesData, INT, depth);

        IMPLEMENT_PROPERTY(MarchingCubesData, POOL_REAL_ARRAY, data);
    }
	
	DECLARE_PUBLIC_PROPERTY(int, width, 8);
	DECLARE_PUBLIC_PROPERTY(int, height, 8);
	DECLARE_PUBLIC_PROPERTY(int, depth, 8);

	DECLARE_PUBLIC_PROPERTY(PoolRealArray, data, {});
};
