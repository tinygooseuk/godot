// stb_perlin.h - v0.2 - perlin noise
// public domain single-file C implementation by Sean Barrett
//
// LICENSE
//
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
//
//
// to create the implementation,
//     #define STB_PERLIN_IMPLEMENTATION
// in *one* C/CPP file that includes this file.


// Documentation:
//
// float  stb_perlin_noise3( float x,
//                           float y,
//                           float z,
//                           int   x_wrap=0,
//                           int   y_wrap=0,
//                           int   z_wrap=0)
//
// This function computes a random value at the coordinate (x,y,z).
// Adjacent random values are continuous but the noise fluctuates
// its randomness with period 1, i.e. takes on wholly unrelated values
// at integer points. Specifically, this implements Ken Perlin's
// revised noise function from 2002.
//
// The "wrap" parameters can be used to create wraparound noise that
// wraps at powers of two. The numbers MUST be powers of two. Specify
// 0 to mean "don't care". (The noise always wraps every 256 due
// details of the implementation, even if you ask for larger or no
// wrapping.)

#pragma once

float stb_perlin_noise3(float x, float y, float z, int x_wrap = 0, int y_wrap = 0, int z_wrap = 0);