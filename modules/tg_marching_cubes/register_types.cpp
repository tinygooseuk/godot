#include "register_types.h"
#include "core/class_db.h"

#include "src/marching_cubes_terrain.h"
#include "src/marching_cubes_data.h"
#include "src/marching_cubes_editor_plugin.h"

void register_tg_marching_cubes_types()
{
    ClassDB::register_class<MarchingCubesTerrain>();
    ClassDB::register_class<MarchingCubesData>();

#ifdef TOOLS_ENABLED
	EditorPlugins::add_by_type<MarchingCubesEditorPlugin>();
#endif
}

void unregister_tg_marching_cubes_types()
{

}
