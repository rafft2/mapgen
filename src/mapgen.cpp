#include "stdio.h"

#include "math.h"
#include "stdlib.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb_image_write.h"
#pragma warning(pop)

#define STB_PERLIN_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb_perlin.h"
#pragma warning(pop)

#include "nom.h"

enum biome_type_id : u16
{
    BIOME_TYPE_NULL = 0,
    BIOME_TYPE_OCEAN,
    BIOME_TYPE_MOUNTAIN,
    BIOME_TYPE_PLAIN,
    BIOME_TYPE_DESERT,
    BIOME_TYPE_JUNGLE,

    BIOME_TYPE_COUNT,
};

struct color_rgb
{
    u8 r, g, b;
};

color_rgb ColorRGB(u8 r, u8 g, u8 b)
{
    color_rgb result = {};
    result.r = r;
    result.g = g;
    result.b = b;
    return(result);
}
color_rgb COLOR_BLACK = ColorRGB(0, 0, 0);
color_rgb COLOR_WHITE = ColorRGB(255, 255, 255);
color_rgb COLOR_GREEN = ColorRGB(0, 255, 0);
color_rgb COLOR_MUSTARD_GREEN = ColorRGB(110, 110, 48);
color_rgb COLOR_BLUE = ColorRGB(0, 0, 255);
color_rgb COLOR_YELLOW = ColorRGB(255, 255, 0);
color_rgb biome_color_table[BIOME_TYPE_COUNT] = { COLOR_BLACK, COLOR_BLUE, COLOR_WHITE, COLOR_GREEN, COLOR_YELLOW, COLOR_MUSTARD_GREEN }; 

struct tile_data
{
    f32 elevation;
    f32 moisture;
};

#include "stdlib.h"
#include "time.h"
f32 RandomFloat(f32 min, f32 max)
{
    f32 result = (f32)rand() / (f32)RAND_MAX;
    result = min + result * (max - min);
    return(result);
}

f32 SampleNoise(f32 x, f32 y, f32 scale)
{
    f32 result = stb_perlin_fbm_noise3(x * scale * 5.0f, y * scale * 5.0f, 0.0f, 2.0f, 0.5f, 8);
    return(result);
}

biome_type_id EvaluateBiome(f32 elevation, f32 moisture, f32 temperature)
{
    if(elevation < 0.2f)
    {
        return(BIOME_TYPE_OCEAN);
    }
    if(elevation > 0.8f)
    {
        return(BIOME_TYPE_MOUNTAIN);
    }
    
    if(temperature > 0.7f && moisture < 0.4f)
    {
        return(BIOME_TYPE_DESERT);
    }
    else
    {
        if(moisture > 0.4f)
        {
            return(BIOME_TYPE_JUNGLE);
        }
        else
        {
            return(BIOME_TYPE_PLAIN);
        }
    }

    return(BIOME_TYPE_NULL);
}

int main(void)
{
    srand((u32)time(NULL));
    s32 map_width = 256; s32 map_height = 256;
    f32 scale = 2.5f / (f32)(map_width);
    f32 max_elevation = 0.0f;
    f32 max_moisture = 0.0f;
    
    tile_data* map_tile_grid = (tile_data*)malloc(sizeof(tile_data) * map_width * map_height);
    for(s32 i = 0; i < map_width; i++)
    {
        for(s32 j = 0; j < map_height; j++)
        {
            map_tile_grid[IDX2D(i, j, map_width)] = {};
            f32 elevation = fabsf(SampleNoise((f32)i, (f32)j, scale));
            map_tile_grid[IDX2D(i, j, map_width)].elevation = elevation;
            if(elevation > max_elevation)
            {
                max_elevation = elevation;
            }

            f32 moisture = fabsf(SampleNoise((f32)i + 1000.0f, (f32)j + 1000.0f, scale * 2.0f));
            map_tile_grid[IDX2D(i, j, map_width)].moisture = moisture;
            if(moisture > max_moisture)
            {
                max_moisture = moisture;
            }
        }
    }

    color_rgb* output_image = (color_rgb*)malloc(sizeof(color_rgb) * map_width * map_height);
    for(s32 i = 0; i < map_width; i++)
    {
        for(s32 j = 0; j < map_height; j++)
        {
            f32 elevation = map_tile_grid[IDX2D(i, j, map_width)].elevation / max_elevation;
            f32 moisture = map_tile_grid[IDX2D(i, j, map_width)].moisture / max_moisture;
            f32 equator = (f32)(map_height - 1) / 2.0f;
            f32 distance_from_equator = fabsf((f32)j - equator) / equator;
            f32 temperature = 1.0f - ((distance_from_equator + elevation) / 2.0f);
            biome_type_id biome = EvaluateBiome(elevation, moisture, temperature);

            output_image[IDX2D(i, j, map_width)] = biome_color_table[biome];
        }
    }

    const char *filename = "output/map.png";
    s32 ok = stbi_write_png(filename, map_width, map_height, 3, (u8*)output_image, 3 * map_width);
    if(!ok)
    {
        printf("error with stbi_write_png.\n");
    }
    else
    {
        printf("wrote image: %s.\n", filename);
    }
    
    return(0);
}