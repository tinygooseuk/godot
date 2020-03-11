#pragma once
#include "core/math/vector3.h"
#include "core/color.h"
// Adapted from https://graphics.stanford.edu/~mdfisher/MarchingCubes.html

namespace MarchingCubes {
struct Face {
	Face() {}
	Face(uint32_t index1, uint32_t index2, uint32_t index3) {
		indices[0] = index1;
		indices[1] = index2;
		indices[2] = index3;
	}

	uint32_t indices[3];
};

// From MarchingCubes.h
struct GridCell {
	Vector3 position[8]; //position of each corner of the grid in world space
	float value[8]; //value of the function at this grid corner
	Color colour[8];
};

//given a grid cell, returns the set of triangles that approximates the region where val == 0.
int polygonise(GridCell &cell, Face *faces, int &new_vertex_count, Vector3 *vertices);
} // namespace MarchingCubes