#ifndef NOISE_H

#define STB_PERLIN_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb_perlin.h"
#pragma warning(pop)

#include "nom.h"

f32 SampleFBM(f32 x, f32 y, s32 seed, s32 octaves, f32 lacunarity, f32 persistence)
{
    f32 result = 0.0f;
    f32 frequency = 1.0f;
    f32 amplitude = 1.0f;
    for(s32 i = 0; i < octaves; i++)
    {
        f32 nx = x * frequency + 0.5f;
        f32 ny = y * frequency + 0.5f;
        f32 nz = (f32)seed * 0.123f + (f32)i * 0.37f;

        result += amplitude * stb_perlin_noise3_seed(nx, ny, nz, 0, 0, 0, seed);

        frequency *= lacunarity;
        amplitude *= persistence;
    }

    // NOTE: the result of FBM in practice is always roughly between [-1.1f, +1.1f]
    //       we put it into the [0, 1] range with clamp to make sure
    result = (result + 1.0f) / 2.0f; // [0, 1]
    result = Clampf(result, 0.0f, 1.0f);
    return(result);
}

f32 SampleWarpedFBM(f32 x, f32 y, s32 seed, s32 octaves, f32 lacunarity, f32 persistence)
{
    f32 warp_strength = 0.5f;
    f32 warp_x = x + stb_perlin_noise3_seed(x + 0.5f, y + 0.5f, (f32)seed * 0.123f + 29.84f, 0, 0, 0, seed+555) * warp_strength;
    f32 warp_y = y + stb_perlin_noise3_seed(x + 13.24f, y + 7.39f, (f32)seed * 0.123f + 61.32f, 0, 0, 0, seed+666) * warp_strength;
    f32 result = SampleFBM(warp_x, warp_y, seed, octaves, lacunarity, persistence);
    return(result);
}
f32 SampleWarpedFBM(f32 x, f32 y, s32 seed)
{
    f32 result = SampleWarpedFBM(x, y, seed, 6, 2.0f, 0.5f);
    return(result);
}

#define NOISE_H
#endif