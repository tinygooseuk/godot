#pragma once
#include "core/math/vector3.h"
// Adapted from https://graphics.stanford.edu/~mdfisher/MarchingCubes.html

// From BaseMesh.h
struct TriMeshFace
{
    TriMeshFace() {}
    TriMeshFace(uint32_t I0, uint32_t I1, uint32_t I2)
    {
        I[0] = I0;
        I[1] = I1;
        I[2] = I2;
    }

    uint32_t I[3];
};


// From MarchingCubes.h
struct GRIDCELL {
   Vector3 p[8];	//position of each corner of the grid in world space
   float val[8];	//value of the function at this grid corner
};

//given a grid cell, returns the set of triangles that approximates the region where val == 0.
int Polygonise(GRIDCELL &Grid, TriMeshFace *Triangles, int &NewVertexCount, Vector3 *Vertices);