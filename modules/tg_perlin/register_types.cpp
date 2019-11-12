#include "register_types.h"

#include "core/class_db.h"
#include "src/perlin_generator.h"
#include "src/perlin_landscape.h"

void register_tg_perlin_types()
{
    ClassDB::register_class<PerlinGenerator>();
	ClassDB::register_class<PerlinLandscape>();
}

void unregister_tg_perlin_types()
{

}
