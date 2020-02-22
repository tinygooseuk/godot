#pragma once

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "editor/pane_drag.h"
#include "marching_cubes_terrain.h"
#include "marching_cubes_editor.h"

#if TOOLS_ENABLED
class MarchingCubesEditorPlugin : public EditorPlugin {

	GDCLASS(MarchingCubesEditorPlugin, EditorPlugin);

	MarchingCubesEditor* marching_cubes_editor;
	EditorNode* editor;

protected:
	void _notification(int p_what);

public:
	virtual bool forward_spatial_gui_input(Camera *p_camera, const Ref<InputEvent> &p_event) { return marching_cubes_editor->forward_spatial_input_event(p_camera, p_event); }
	virtual String get_name() const { return "MarchingCubes"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object* p_object);
	virtual bool handles(Object* p_object) const;
	virtual void make_visible(bool p_visible);

	MarchingCubesEditorPlugin(EditorNode* p_node);
	~MarchingCubesEditorPlugin();
};
#endif // TOOLS_ENABLED