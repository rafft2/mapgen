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
    BIOME_TYPE_MARSH,
    BIOME_TYPE_FOREST,
    BIOME_TYPE_TUNDRA,

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
color_rgb COLOR_PURPLE = ColorRGB(127, 0, 127);
color_rgb COLOR_DARK_GREEN = ColorRGB(0, 127, 0);
color_rgb COLOR_LIGHT_BLUE = ColorRGB(127, 127, 255);
color_rgb biome_color_table[BIOME_TYPE_COUNT] = { COLOR_BLACK, COLOR_BLUE, COLOR_WHITE,
                                                 COLOR_GREEN, COLOR_YELLOW, COLOR_MUSTARD_GREEN,
                                                 COLOR_PURPLE, COLOR_DARK_GREEN, COLOR_LIGHT_BLUE };
char *biome_name_table[BIOME_TYPE_COUNT] = { "The void", "Ocean", "Mountain", "Plain", "Desert", "Jungle", "Marsh", "Forest", "Tundra" };

struct tile_data
{
    f32 elevation;
    f32 moisture;
};

#include "stdlib.h"
#include "time.h"
f32 RandomFloat(f32 min, f32 max)
{
    f32 result = (f32)rand() / RAND_MAX;
    result = min + result * (max - min);
    return(result);
}

s32 RandomInt(s32 min, s32 max)
{
    f64 r = rand();
    s32 result = (((s32)fabs(r)) % (max - min + 1)) + min;
    return(result);
}

f32 SampleNoise(f32 x, f32 y, f32 scale)
{
    f32 result = stb_perlin_fbm_noise3(x * scale * 5.0f, y * scale * 5.0f, 0.0f, 2.0f, 0.5f, 8);
    return(result);
}

biome_type_id EvaluateBiome(f32 elevation, f32 moisture, f32 temperature)
{
    if(elevation < 0.15f)
    {
        return(BIOME_TYPE_OCEAN);
    }
    if(elevation > 0.9f)
    {
        return(BIOME_TYPE_MOUNTAIN);
    }
    else if(elevation > 0.5f)
    {
        if(moisture > 0.2f)
        {
            return(BIOME_TYPE_TUNDRA);
        }
        else
        {
            return(BIOME_TYPE_MOUNTAIN);
        }
    }
    else
    {
        if(temperature > 0.7f)
        {
            if(moisture > 0.2f)
            {
                return(BIOME_TYPE_JUNGLE);
            }
            else
            {
                return(BIOME_TYPE_DESERT);
            }
        }
        else
        {
            if(moisture > 0.5f)
            {
                return(BIOME_TYPE_MARSH);
            }
            else if(moisture > 0.2f)
            {
                return(BIOME_TYPE_FOREST);
            }
            else
            {
                return(BIOME_TYPE_PLAIN);
            }
        }
    }

    return(BIOME_TYPE_NULL);
}

vec2i RandomMapPosition(s32 map_width, s32 map_height)
{
    vec2i result = {};
    result.x = RandomInt(0, map_width);
    result.y = RandomInt(0, map_height);
    return(result);
}

s32 ComputeDistanceInTiles(s32 ax, s32 ay, s32 bx, s32 by)
{
    s32 x = ax - bx;
    s32 y = ay - by;
    s32 dist = (s32)sqrtf((f32)(x*x + y*y));
    return(dist);
}

s32 ComputeDistanceInTiles(s32 ax, s32 ay, vec2i p)
{
    s32 dist = ComputeDistanceInTiles(ax, ay, p.x, p.y);
    return(dist);
}

void PrintLineEveryN(s32 i, s32 N)
{
    if((i+1) % N == 0) { printf("\n"); }
}

s32 GetMinDistanceFromLand(s32 ax, s32 ay, vec2i *continent_centers, s32 continent_count)
{
    s32 min_distance_from_land = INT_MAX;
    for(s32 idx = 0; idx < continent_count; idx++)
    {
        s32 dist = ComputeDistanceInTiles(ax, ay, continent_centers[idx]);
        if(dist < min_distance_from_land)
        {
            min_distance_from_land = dist;
        }
    }
    return(min_distance_from_land);
}

int main(void)
{
    srand((u32)time(NULL));
    s32 map_width = 512; s32 map_height = 512;
    f32 scale = 2.5f / (f32)(map_width);
    f32 max_elevation = 0.0f;
    f32 max_moisture = 0.0f;
    s32 biome_stat_table[BIOME_TYPE_COUNT] = {};
    s32 continent_count = RandomInt(1, 8);
    vec2i *continent_centers = (vec2i*)malloc(sizeof(vec2i) * continent_count);
    s32 continent_idx = 0;
    continent_centers[continent_idx++] = RandomMapPosition(map_width, map_height);
    while(continent_idx < continent_count)
    {
        s32 max_distance_in_tiles = 0;
        s32 best_x = 0;
        s32 best_y = 0;
        for(s32 x = 0; x < map_width; x++)
        {
            for(s32 y = 0; y < map_height; y++)
            {
                s32 distance_from_bottom = y;
                s32 distance_from_top = map_height - y;
                s32 distance_from_left = x;
                s32 distance_from_right = map_width - x;
                s32 min_distance_from_land = GetMinDistanceFromLand(x, y, continent_centers, continent_count);
                s32 min = MIN(distance_from_bottom, MIN(distance_from_top, MIN(distance_from_left, MIN(distance_from_right, min_distance_from_land))));
                if(min > max_distance_in_tiles)
                {
                    max_distance_in_tiles = min;
                    best_x = x;
                    best_y = y;
                }
            }
        }
        continent_centers[continent_idx++] = {best_x, best_y};
    }

    printf("Continent count: %d.\n", continent_count);
    for(s32 i = 0; i < continent_count; i++)
    {
        printf("Continent %d center: %d %d.  ", i, continent_centers[i].x, continent_centers[i].y);
        PrintLineEveryN(i, 3);
    }
    
    tile_data* map_tile_grid = (tile_data*)malloc(sizeof(tile_data) * map_width * map_height);
    // TODO: probably the loop variable names to x and y
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
            f32 denom = continent_count > 2 ? 0.5f*powf(2.0f, (f32)((s32)sqrtf((f32)continent_count + 1))) : 2.0f;
            f32 max_distance = ((f32)map_width) * (sqrtf(2.0f) / denom);
            f32 min_distance_from_land = (f32)GetMinDistanceFromLand(i, j, continent_centers, continent_count);
            
            // distance in [0, 1]
            f32 normalized_distance = min_distance_from_land / max_distance;
            f32 continentality = 1.0f - normalized_distance;
            // 3 hours on desmos produced this abomination
            elevation = continentality * elevation * expf(0.1f * Clampf(continentality - 0.5f, 0.0f, 1.0f) / (1.0f - continentality));

            f32 moisture = map_tile_grid[IDX2D(i, j, map_width)].moisture / max_moisture;
            f32 equator = (f32)(map_height - 1) / 2.0f;
            f32 distance_from_equator = fabsf((f32)j - equator) / equator;
            f32 temperature = 1.0f - ((distance_from_equator + elevation) / 2.0f);
            biome_type_id biome = EvaluateBiome(elevation, moisture, temperature);

            biome_stat_table[biome]++;
            output_image[IDX2D(i, j, map_width)] = biome_color_table[biome];
        }
    }

    printf("\n========= MAP RESULTS =========\n");
    for(s32 i = 0; i < BIOME_TYPE_COUNT; i++)
    {
        printf("%s: %d (%.1f%%).  ", biome_name_table[i], biome_stat_table[i], (f32)biome_stat_table[i] * 100.0f / (f32)(map_width * map_height));
        PrintLineEveryN(i, 3);
    }
    printf("\n\n");

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