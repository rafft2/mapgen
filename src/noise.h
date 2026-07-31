#ifndef NOISE_H

#define STB_PERLIN_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb_perlin.h"
#pragma warning(pop)

#include "nom.h"

#define PHI32 1.618034f

inline f32 Perlin3D(f32 nx, f32 ny, f32 nz, s32 seed)
{
    f32 result = stb_perlin_noise3_seed(nx + PHI32, ny + PHI32, nz + PHI32, 0, 0, 0, seed);
    return(result);
}
inline f32 Perlin2D(f32 nx, f32 ny, s32 seed)
{
    f32 result = Perlin3D(nx, ny, 0.0f, seed);
    return(result);
}
inline f32 Perlin1D(f32 nx, s32 seed)
{
    f32 result = Perlin2D(nx, 0.0f, seed);
    return(result);
}

inline f32 SampleFBM(f32 x, f32 y, s32 seed, s32 octaves, f32 lacunarity, f32 persistence)
{
    f32 result = 0.0f;
    f32 frequency = 1.0f;
    f32 amplitude = 1.0f;
    f32 tot_amplitude = 0.0f;
    for(s32 i = 0; i < octaves; i++)
    {
        f32 nx = x * frequency;
        f32 ny = y * frequency;

        result += amplitude * Perlin2D(nx, ny, seed);
        tot_amplitude += amplitude;

        frequency *= lacunarity;
        amplitude *= persistence;
    }
    result = (result + 1.0f) / tot_amplitude;
    return(result);
}
inline f32 SampleFBM(f32 x, f32 y, s32 seed)
{
    f32 result = SampleFBM(x, y, seed, 6, 2.0f, 0.5f);
    return(result);
}

inline f32 SampleWarpedFBM(f32 x, f32 y, s32 seed, s32 octaves, f32 lacunarity, f32 persistence, f32 warp_strength)
{
    f32 warp_x = x + Perlin2D(x, y, seed) * warp_strength;
    f32 warp_y = y + Perlin2D(x + 0.5f, y + 0.5f, seed) * warp_strength;
    f32 result = SampleFBM(warp_x, warp_y, seed, octaves, lacunarity, persistence);
    return(result);
}
inline f32 SampleWarpedFBM(f32 x, f32 y, s32 seed, s32 octaves, f32 lacunarity, f32 persistence)
{
    f32 result = SampleWarpedFBM(x, y, seed, octaves, lacunarity, persistence, 0.5f);
    return(result);
}
inline f32 SampleWarpedFBM(f32 x, f32 y, s32 seed)
{
    f32 result = SampleWarpedFBM(x, y, seed, 6, 2.0f, 0.5f);
    return(result);
}
inline f32 SampleWarpedFBM(f32 x, f32 y, s32 seed, f32 warp_strength)
{
    f32 result = SampleWarpedFBM(x, y, seed, 6, 2.0f, 0.5f, warp_strength);
    return(result);
}

f32 GetNoiseScaleForLargeDetails(s32 map_width, s32 map_height)
{
    f32 result = 1.0f / (sqrtf((f32)MAX(map_width, map_height)) * PI32);
    return(result);
}

f32 GetNoiseScaleForMediumDetails(s32 map_width, s32 map_height)
{
    f32 result = 1.0f / (sqrtf((f32)MAX(map_width, map_height)) * PHI32);
    return(result);
}

f32 GetNoiseScaleForSmallDetails(s32 map_width, s32 map_height)
{
    f32 result = 1.0f / (sqrtf((f32)MAX(map_width, map_height)));
    return(result);
}


#define NOISE_H
#endif