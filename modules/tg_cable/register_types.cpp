#include "register_types.h"

#include "core/class_db.h"
#include "src/floppy_cable.h"

void register_tg_cable_types()
{
    ClassDB::register_class<FloppyCable3D>();
}

void unregister_tg_cable_types()
{

}
