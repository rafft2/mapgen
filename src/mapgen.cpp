#include "stdio.h"

#include "math.h"
#include "stdlib.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb_image_write.h"
#pragma warning(pop)

#include "nom.h"
#include "noise.h"

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
    BIOME_TYPE_HIGHLAND,
    BIOME_TYPE_SAVANNAH,
    BIOME_TYPE_SHALLOW_OCEAN,
    BIOME_TYPE_SNOW,
    BIOME_TYPE_RAINFOREST,

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
color_rgb COLOR_DARK_BLUE = ColorRGB(0, 0, 255);
color_rgb COLOR_YELLOW = ColorRGB(255, 255, 0);
color_rgb COLOR_PURPLE = ColorRGB(127, 0, 127);
color_rgb COLOR_DARK_GREEN = ColorRGB(0, 127, 0);
color_rgb COLOR_LIGHT_BLUE = ColorRGB(127, 127, 255);
color_rgb COLOR_DARK_GREY = ColorRGB(50, 40, 30);
color_rgb COLOR_GREY = ColorRGB(127, 127, 127);
color_rgb COLOR_ORANGE = ColorRGB(200, 160, 140);
color_rgb COLOR_BLUE = ColorRGB(50, 50, 255);
color_rgb COLOR_AQUA = ColorRGB(0, 160, 180);
color_rgb biome_color_table[BIOME_TYPE_COUNT] = { COLOR_BLACK, COLOR_DARK_BLUE, COLOR_GREY,
                                                  COLOR_GREEN, COLOR_YELLOW, COLOR_MUSTARD_GREEN,
                                                  COLOR_PURPLE, COLOR_DARK_GREEN, COLOR_LIGHT_BLUE,
                                                  COLOR_DARK_GREY, COLOR_ORANGE, COLOR_BLUE,
                                                  COLOR_WHITE, COLOR_AQUA };
char *biome_name_table[BIOME_TYPE_COUNT] = { "The void", "Ocean", "Mountain", "Plain", "Desert", "Jungle", "Marsh", "Forest", "Tundra", "Highlands", "Savannah", 
                                             "Shallow Ocean", "Snow", "Rainforest" };

#include "stdlib.h"
#include "time.h"
#include "float.h"
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

biome_type_id EvaluateBiome(f32 elevation, f32 moisture, f32 temperature)
{
    if(elevation < 0.45f) { return(BIOME_TYPE_OCEAN); }
    if(elevation < 0.46f) { return(BIOME_TYPE_SHALLOW_OCEAN); }

    if(elevation > 0.9f) { return(BIOME_TYPE_SNOW); }
    if(elevation > 0.8f) { return(BIOME_TYPE_MOUNTAIN); }

    if(elevation > 0.6f)
    {
        if(temperature > 0.5f)
        {
            return(BIOME_TYPE_HIGHLAND);
        }
        else
        {
            if(moisture > 0.5f)
            {
                return(BIOME_TYPE_TUNDRA);
            }
            else
            {
                return(BIOME_TYPE_MOUNTAIN);
            }
        }
    }
    else
    {
        if(temperature > 0.95f)
        {
            return(BIOME_TYPE_DESERT);
        }
        else if(temperature > 0.65f)
        {
            if(moisture > 0.65f)
            {
                return(BIOME_TYPE_JUNGLE);
            }
            else if(moisture > 0.5f)
            {
                return(BIOME_TYPE_SAVANNAH);
            }
            else
            {
                return(BIOME_TYPE_DESERT);
            }
        }
        else
        {
            if(moisture > 0.8f)
            {
                return(BIOME_TYPE_MARSH);
            }
            else if(moisture > 0.6f)
            {
                return(BIOME_TYPE_RAINFOREST);
            }
            else if(moisture > 0.4f)
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

enum direction : u8
{
    DIRECTION_NONE = 0,
    SOUTH,
    SOUTH_WEST,
    WEST,
    NORTH_WEST,
    NORTH,
    NORTH_EAST,
    EAST,
    SOUTH_EAST,

    DIRECTION_COUNT
};

vec2i adjacent_tile_from_direction[DIRECTION_COUNT] = { {0, 0}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1} };

#define CRUST_TYPE_CONTINENTAL 0u
#define CRUST_TYPE_OCEANIC 1u

struct plate_data
{
    vec2i center;
    u8 crust_type;
 
    f32 base_elevation;
    f32 base_moisture;
    f32 base_temperature;
};

struct tile_data
{    
    u32 plate_index[3]; // Closest 3 plates for blending

    f32 elevation;
    f32 moisture;
    f32 temperature;

    u8 water_descent_direction; // TODO: should water fall towards multiple directions?
    s32 water_accumulation_count;

};

void GenerateAndAssignPlates(tile_data *map_tile_grid, s32 map_width, s32 map_height, plate_data *plates, s32 plate_count, s32 seed)
{
    f32 noise_scale = 16.0f / (f32)(map_width);
    for(s32 i = 0; i < plate_count; i++)
    {
        vec2i center = { RandomInt(4, map_width - 4), RandomInt(4, map_height - 4) };
        s32 roll = RandomInt(1, 8);
        u8 crust_type = roll == 8 ? CRUST_TYPE_CONTINENTAL : CRUST_TYPE_OCEANIC;

        f32 nx = (f32)center.x * noise_scale;
        f32 ny = (f32)center.y * noise_scale;

        f32 elevation_offset = crust_type == CRUST_TYPE_OCEANIC ? 0.15f : 0.55f;
        f32 base_elevation = elevation_offset + (SampleWarpedFBM(nx, ny, seed) / 2.0f);

        // TODO: offset moisture based on adjacent plates
        //       more oceanic adjacent plates = higher base moisture and viceversa.
        //       Should probably split center and crust type assignment and noise generation.
        f32 base_moisture = SampleWarpedFBM(nx + 17.51f, ny + 31.82f, seed);

        f32 equator = (f32)(map_height - 1) / 2.0f;
        f32 distance_from_equator = fabsf((f32)center.y - equator) / equator; // [0, 1]
        f32 temperature_offset = (1.0f - distance_from_equator) / 2.0f; // [0, 0.5]
        f32 base_temperature = temperature_offset + (SampleWarpedFBM(nx + 7.34f, ny + 3.227f, seed) / 2.0f);

        plates[i].center = center;
        plates[i].crust_type = crust_type;
        plates[i].base_elevation = base_elevation;
        plates[i].base_moisture = base_moisture;
        plates[i].base_temperature = base_temperature;
    }
    
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            s32 closest_dist[3] = {INT_MAX, INT_MAX, INT_MAX};
            u32 closest_plates[3] = {0, 1, 2};
            for(s32 i = 0; i < plate_count; i++)
            {
                s32 distance = ComputeDistanceInTiles(x, y, plates[i].center);
                s32 j = 2;
                while(j >= 0 && distance < closest_dist[j])
                {
                    j--;
                }
                j++;
                if(j <= 2 && distance < closest_dist[j])
                {
                    closest_dist[j] = distance;
                    closest_plates[j] = (u32)i;
                }
            }
            map_tile_grid[IDX2D(x, y, map_width)].plate_index[0] = closest_plates[0];
            map_tile_grid[IDX2D(x, y, map_width)].plate_index[1] = closest_plates[1];
            map_tile_grid[IDX2D(x, y, map_width)].plate_index[2] = closest_plates[2];
            
        }
    }
}

void GenerateTiles(tile_data *map_tile_grid, s32 map_width, s32 map_height, plate_data *plates, s32 seed)
{
    f32 noise_scale = 16.0f / (f32)(map_width);
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            if(x == 200 && y == 250)
            {
                s32 s = 29392;
            }
            f32 nx = (f32)x * noise_scale;
            f32 ny = (f32)y * noise_scale;
            
            u32 *closest_plates = &map_tile_grid[IDX2D(x, y, map_width)].plate_index[0];
            plate_data plate0 = plates[closest_plates[0]];
            plate_data plate1 = plates[closest_plates[1]];
            plate_data plate2 = plates[closest_plates[2]];
            vec2i center0 = plate0.center;
            vec2i center1 = plate1.center;
            vec2i center2 = plate2.center;
            s32 distance0 = ComputeDistanceInTiles(x, y, center0);
            s32 distance1 = ComputeDistanceInTiles(x, y, center1);
            s32 distance2 = ComputeDistanceInTiles(x, y, center2);
            f32 w0 = 1.0f / ((f32)(distance0 * distance0) + EPSILON32);
            f32 w1 = 1.0f / ((f32)(distance1 * distance1) + EPSILON32);
            f32 w2 = 1.0f / ((f32)(distance2 * distance2) + EPSILON32);
            f32 total_weight = w0 + w1 + w2;
            w0 /= total_weight;
            w1 /= total_weight;
            w2 /= total_weight;

            f32 blended_base_elevation = w0 * plate0.base_elevation +
                                         w1 * plate1.base_elevation +
                                         w2 * plate2.base_elevation;
            
            f32 blended_base_moisture =  w0 * plate0.base_moisture +
                                         w1 * plate1.base_moisture +
                                         w2 * plate2.base_moisture;

            f32 blended_base_temperature = w0 * plate0.base_temperature +
                                           w1 * plate1.base_temperature +
                                           w2 * plate2.base_temperature;

            f32 elevation = blended_base_elevation * 0.65f + SampleWarpedFBM(nx + 1.11f, ny + 2.788f, seed) * 0.35f;
            f32 moisture = blended_base_moisture * 0.5f + SampleWarpedFBM(nx + 5.56f, ny + 1.31f, seed) * 0.5f;
            f32 temperature = blended_base_temperature * 0.75f + SampleWarpedFBM(nx + 3.422f, ny + 13.2f, seed) * 0.25f;

            map_tile_grid[IDX2D(x, y, map_width)].elevation = elevation;
            map_tile_grid[IDX2D(x, y, map_width)].moisture = moisture;
            map_tile_grid[IDX2D(x, y, map_width)].temperature = temperature;
        }
    }
}

int main(void)
{
    s32 seed = (s32)time(NULL);
    srand((u32)seed);

    s32 map_width = 1024; s32 map_height = 512;
    tile_data* map_tile_grid = (tile_data*)calloc((u32)(map_width * map_height), sizeof(tile_data));
    
    s32 plate_count = 256;
    plate_data *plates = (plate_data*)malloc(sizeof(plate_data) * plate_count);
    GenerateAndAssignPlates(map_tile_grid, map_width, map_height, plates, plate_count, seed);
    GenerateTiles(map_tile_grid, map_width, map_height, plates, seed);

    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            f32 elev = map_tile_grid[IDX2D(x, y, map_width)].elevation;
            u8 steepest_dir = DIRECTION_NONE;
            f32 max_diff = 0.0f;
            for(u8 dir = 0; dir < DIRECTION_COUNT; dir++)
            {
                vec2i pos = {x, y};
                vec2i p_adjacent = pos + adjacent_tile_from_direction[dir];
                if(IN_BOUNDS2D(p_adjacent.x, p_adjacent.y, map_width, map_height))
                {
                    f32 adj_elev = map_tile_grid[IDX2D(p_adjacent.x, p_adjacent.y, map_width)].elevation;
                    f32 diff = elev - adj_elev;
                    if(diff > max_diff)
                    {
                        max_diff = diff;
                        steepest_dir = dir;
                    }
                } 
            }
            map_tile_grid[IDX2D(x, y, map_width)].water_descent_direction = steepest_dir;
        }
    }

    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            vec2i current_pos = { x, y };
            u8 current_dir = map_tile_grid[IDX2D(x, y, map_width)].water_descent_direction;
            while(current_dir != DIRECTION_NONE)
            { 
                vec2i movement = adjacent_tile_from_direction[current_dir];
                current_pos = current_pos + movement;

                // It should be illegal to be out of bounds here
                ASSERT(IN_BOUNDS2D(current_pos.x, current_pos.y, map_width, map_height));
                map_tile_grid[IDX2D(current_pos.x, current_pos.y, map_width)].water_accumulation_count += 1;
                current_dir = map_tile_grid[IDX2D(current_pos.x, current_pos.y, map_width)].water_descent_direction;
            }
        }
    }

    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            s32 accumulation = map_tile_grid[IDX2D(x, y, map_width)].water_accumulation_count;
            f32 value = 0.0f;
            if(accumulation > 100)
            {
                value = 0.4f;
            }
            else if(accumulation > 50)
            {
                value = 0.2f;
            }
            map_tile_grid[IDX2D(x, y, map_width)].elevation -= value;
            map_tile_grid[IDX2D(x, y, map_width)].elevation = Clampf(map_tile_grid[IDX2D(x, y, map_width)].elevation, 0.0f, 1.0f);
        }
    }

    f32 max_elevation = FLT_MIN / 2.0f; f32 min_elevation = FLT_MAX / 2.0f;
    f32 max_moisture = FLT_MIN / 2.0f; f32 min_moisture = FLT_MAX / 2.0f;
    f32 max_temperature = FLT_MIN / 2.0f; f32 min_temperature = FLT_MAX / 2.0f;
    s32 max_accumulation = 0; s32 min_accumulation = INT_MAX;
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            tile_data tile = map_tile_grid[IDX2D(x, y, map_width)];
            max_elevation = MAX(max_elevation, tile.elevation);
            min_elevation = MIN(min_elevation, tile.elevation);
            max_moisture = MAX(max_moisture, tile.moisture);
            min_moisture = MIN(min_moisture, tile.moisture);
            max_temperature = MAX(max_temperature, tile.temperature);
            min_temperature = MIN(min_temperature, tile.temperature);
            max_accumulation = MAX(max_accumulation, tile.water_accumulation_count);
            if(tile.water_accumulation_count > 50)
            {
                min_accumulation = MIN(min_accumulation, tile.water_accumulation_count);
            }
        }
    }

    u8* plates_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    u8* elevation_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    u8* moisture_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    u8* temperature_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    u8* accumulation_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    s32 biome_stat_table[BIOME_TYPE_COUNT] = {};
    color_rgb* output_image = (color_rgb*)malloc(sizeof(color_rgb) * map_width * map_height);
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            f32 elevation = map_tile_grid[IDX2D(x, y, map_width)].elevation;
            f32 moisture = map_tile_grid[IDX2D(x, y, map_width)].moisture;
            f32 temperature = map_tile_grid[IDX2D(x, y, map_width)].temperature;

            ASSERT(elevation >= 0.0f && elevation <= 1.0f);
            ASSERT(moisture >= 0.0f && moisture <= 1.0f);
            ASSERT(temperature >= 0.0f && temperature <= 1.0f);

            // NOTE: a little extra noise to reduce straight lines
            f32 jitter_strength = 0.02f;
            moisture = moisture + stb_perlin_noise3_seed((f32)x * 10.0f + 2.5f, (f32)y * 10.0f + 4.5f, (f32)seed * 0.445f + 3.71f, 0, 0, 0, seed+222) * jitter_strength;
            temperature = temperature + stb_perlin_noise3_seed((f32)x * 10.0f + 2.134f, (f32)y * 10.0f + 13.14f, (f32)seed * 0.251f + 1.32f, 0, 0, 0, seed+333) * jitter_strength;
            
            biome_type_id biome = EvaluateBiome(elevation, moisture, temperature);

            biome_stat_table[biome]++;
            output_image[IDX2D(x, y, map_width)] = biome_color_table[biome];

            u32 plate_index = map_tile_grid[IDX2D(x, y, map_width)].plate_index[0];
            plates_map[IDX2D(x, y, map_width)] = plates[plate_index].crust_type == CRUST_TYPE_OCEANIC ? 0u : 255u;

            s32 accumulation = map_tile_grid[IDX2D(x, y, map_width)].water_accumulation_count;
            f32 grayscale = 0.0f;
            if(accumulation > 50)
            {
                grayscale = (f32)(accumulation - min_accumulation) / (f32)(max_accumulation - min_accumulation);
            }
            accumulation_map[IDX2D(x, y, map_width)] = (u8)(grayscale * 255.0f);
        
            grayscale = (elevation - min_elevation) / (max_elevation - min_elevation);
            elevation_map[IDX2D(x, y, map_width)] = (u8)(grayscale * 255.0f);

            grayscale = (moisture - min_moisture) / (max_moisture - min_moisture);
            moisture_map[IDX2D(x, y, map_width)] = (u8)(grayscale * 255.0f);

            grayscale = (temperature - min_temperature) / (max_temperature - min_temperature);
            temperature_map[IDX2D(x, y, map_width)] = (u8)(grayscale * 255.0f);
        }
    }

    ASSERT(stbi_write_png("output/map.png", map_width, map_height, 3, (u8*)output_image, 3 * map_width));
    ASSERT(stbi_write_png("output/plates.png", map_width, map_height, 1, plates_map, 1 * map_width));
    ASSERT(stbi_write_png("output/elevation.png", map_width, map_height, 1, elevation_map, 1 * map_width));
    ASSERT(stbi_write_png("output/moisture.png", map_width, map_height, 1, moisture_map, 1 * map_width));
    ASSERT(stbi_write_png("output/temperature.png", map_width, map_height, 1, temperature_map, 1 * map_width));
    ASSERT(stbi_write_png("output/accumulation.png", map_width, map_height, 1, accumulation_map, 1 * map_width));
    
    printf("\n========= MAP RESULTS =========\n");
    printf("Seed: %d.\n", seed);
    for(s32 i = 0; i < BIOME_TYPE_COUNT; i++)
    {
        printf("%s: %d (%.1f%%).  ", biome_name_table[i], biome_stat_table[i], (f32)biome_stat_table[i] * 100.0f / (f32)(map_width * map_height));
        PrintLineEveryN(i, 3);
    }
    printf("\n========= STATS =========\n");

    return(0);
}