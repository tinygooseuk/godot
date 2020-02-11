#include "register_types.h"

#include "core/class_db.h"
#include "src/marching_cubes_terrain.h"
#include "src/marching_cubes_data.h"

void register_tg_marching_cubes_types()
{
    ClassDB::register_class<MarchingCubesTerrain>();
    ClassDB::register_class<MarchingCubesData>();
}

void unregister_tg_marching_cubes_types()
{

}
