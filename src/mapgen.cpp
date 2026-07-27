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
    BIOME_TYPE_DEEP_OCEAN,
    BIOME_TYPE_OCEAN,
    BIOME_TYPE_SHALLOW_OCEAN,
    BIOME_TYPE_MOUNTAIN,
    BIOME_TYPE_PLAIN,
    BIOME_TYPE_DESERT,
    BIOME_TYPE_JUNGLE,
    BIOME_TYPE_MARSH,
    BIOME_TYPE_FOREST,
    BIOME_TYPE_TUNDRA,
    BIOME_TYPE_HIGHLAND,
    BIOME_TYPE_SAVANNAH,
    BIOME_TYPE_SNOW,
    BIOME_TYPE_RAINFOREST,
    BIOME_TYPE_TAIGA,
    BIOME_TYPE_ICE,

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
color_rgb COLOR_VERY_DARK_BLUE = ColorRGB(0, 0, 255);
color_rgb COLOR_DARK_BLUE = ColorRGB(25, 25, 255);
color_rgb COLOR_BLUE = ColorRGB(50, 50, 255);
color_rgb COLOR_BLUE_CREEK = ColorRGB(80, 80, 255);
color_rgb COLOR_LIGHT_BLUE = ColorRGB(100, 100, 255);
color_rgb COLOR_AQUA = ColorRGB(0, 160, 180);
color_rgb COLOR_LIGHT_AQUA = ColorRGB(20, 180, 200);
color_rgb COLOR_DARK_GREEN = ColorRGB(0, 127, 0);
color_rgb COLOR_GREEN = ColorRGB(0, 255, 0);
color_rgb COLOR_MUSTARD_GREEN = ColorRGB(110, 110, 48);
color_rgb COLOR_YELLOW = ColorRGB(255, 255, 0);
color_rgb COLOR_PURPLE = ColorRGB(127, 0, 127);
color_rgb COLOR_DARK_GREY = ColorRGB(50, 40, 30);
color_rgb COLOR_GREY = ColorRGB(127, 127, 127);
color_rgb COLOR_ORANGE = ColorRGB(200, 160, 140);
color_rgb COLOR_BROWN = ColorRGB(120, 65, 65);
color_rgb biome_color_table[BIOME_TYPE_COUNT] = { COLOR_BLACK, COLOR_VERY_DARK_BLUE, COLOR_DARK_BLUE, COLOR_BLUE,
                                                  COLOR_GREY, COLOR_GREEN, COLOR_YELLOW, COLOR_MUSTARD_GREEN,
                                                  COLOR_PURPLE, COLOR_DARK_GREEN, COLOR_LIGHT_BLUE, COLOR_DARK_GREY,
                                                  COLOR_ORANGE, COLOR_WHITE, COLOR_AQUA, COLOR_BROWN, COLOR_LIGHT_AQUA };
char *biome_name_table[BIOME_TYPE_COUNT] = { "The void", "Deep Ocean", "Ocean", "Shallow Ocean", "Mountain", "Plain", "Desert", "Jungle", "Marsh",
                                             "Forest", "Tundra", "Highlands", "Savannah", "Snow", "Rainforest", "Taiga", "Ice" };

biome_type_id EvaluateBiome(f32 elevation, f32 moisture, f32 temperature)
{
    if(elevation < 0.35f) { return(BIOME_TYPE_DEEP_OCEAN); }
    if(elevation < 0.475f) { return(BIOME_TYPE_OCEAN); }
    if(elevation < 0.5f) { return(BIOME_TYPE_SHALLOW_OCEAN); }
    if(elevation > 0.9f || temperature <= 0.05f) { return(BIOME_TYPE_SNOW); }
    if(elevation > 0.75f) { return(BIOME_TYPE_MOUNTAIN); }
    if(elevation > 0.6f && temperature > 0.7f) { return(BIOME_TYPE_HIGHLAND); }

    if(temperature >= 0.65f)
    {
        if(moisture >= 0.6f) { return(BIOME_TYPE_JUNGLE); }
        else if(moisture >= 0.4f) { return(BIOME_TYPE_SAVANNAH); }
        else { return(BIOME_TYPE_DESERT); }
    }
    else if(temperature >= 0.4f)
    {
        if(moisture >= 0.6f) { return(BIOME_TYPE_RAINFOREST); }
        else if(moisture >= 0.4f) { return(BIOME_TYPE_FOREST); }
        else { return(BIOME_TYPE_PLAIN); }
    }
    else
    {
        if(moisture >= 0.6f) { return(BIOME_TYPE_MARSH); }
        else if(moisture >= 0.4f) { return(BIOME_TYPE_TAIGA); }
        else { return(BIOME_TYPE_TUNDRA); }
    }

    return(BIOME_TYPE_NULL);
}

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
 };

enum tile_has_river_type : u8
{
    TILE_HAS_NO_RIVER = 0,
    TILE_HAS_MAJOR_RIVER,
    TILE_HAS_STREAM,
    TILE_HAS_CREEK,

    TILE_HAS_RIVER_COUNT
};

struct tile_data
{    
    u32 closest_plate_index;

    f32 elevation;
    f32 moisture;
    f32 temperature;

    u8 water_descent_direction; // TODO: should water fall towards multiple directions?
    s32 water_accumulation_count;
    tile_has_river_type river_type;
};

void GeneratePlatesAndAssignBaseElevationToTiles(tile_data *map_tile_grid, s32 map_width, s32 map_height, plate_data *plates, s32 plate_count)
{
    for(s32 i = 0; i < plate_count; i++)
    {
        plates[i].center = { RandomInt(4, map_width - 4), RandomInt(4, map_height - 4) };
        
        s32 roll = RandomInt(1, 4);
        u8 crust_type = roll == 1 ? CRUST_TYPE_CONTINENTAL : CRUST_TYPE_OCEANIC;
        plates[i].crust_type = crust_type;
        
        plates[i].base_elevation = crust_type == CRUST_TYPE_OCEANIC ? 0.05f : 0.3f;
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
                
                // TODO: there's probably a better way to do this
                s32 j = 2;
                while(j >= 0 && distance < closest_dist[j])
                {
                    j--;
                }
                j++;
                if(j == 2)
                {
                    closest_dist[j] = distance;
                    closest_plates[j] = (u32)i;
                }
                else if(j == 1)
                {
                    s32 tmp_dist = closest_dist[1];
                    u32 tmp_index = closest_plates[1];
                    closest_dist[1] = distance;
                    closest_plates[1] = (u32)i;
                    closest_dist[2] = tmp_dist;
                    closest_plates[2] = tmp_index;
                }
                else if(j == 0)
                {
                    s32 tmp_dist0 = closest_dist[0];
                    u32 tmp_index0 = closest_plates[0];
                    s32 tmp_dist1 = closest_dist[1];
                    u32 tmp_index1 = closest_plates[1];
                    closest_dist[0] = distance;
                    closest_plates[0] = (u32)i;
                    closest_dist[1] = tmp_dist0;
                    closest_plates[1] = tmp_index0;
                    closest_dist[2] = tmp_dist1;
                    closest_plates[2] = tmp_index1;
                }
            }
            map_tile_grid[IDX2D(x, y, map_width)].closest_plate_index = closest_plates[0];

            plate_data plate0 = plates[closest_plates[0]];
            plate_data plate1 = plates[closest_plates[1]];
            plate_data plate2 = plates[closest_plates[2]];
            s32 distance0 = ComputeDistanceInTiles(x, y, plate0.center);
            s32 distance1 = ComputeDistanceInTiles(x, y, plate1.center);
            s32 distance2 = ComputeDistanceInTiles(x, y, plate2.center);
            f32 w0 = 1.0f / ((f32)(distance0 * distance0)*PI32 + EPSILON32);
            f32 w1 = 1.0f / ((f32)(distance1 * distance1)*PI32 + EPSILON32);
            f32 w2 = 1.0f / ((f32)(distance2 * distance2)*PI32 + EPSILON32);
            f32 total_weight = w0 + w1 + w2;
            w0 /= total_weight;
            w1 /= total_weight;
            w2 /= total_weight;
            f32 elevation = w0 * plate0.base_elevation + w1 * plate1.base_elevation + w2 * plate2.base_elevation;
            map_tile_grid[IDX2D(x, y, map_width)].elevation = elevation;
        }
    }
}

void GenerateNoiseMap(tile_data *map_tile_grid, s32 map_width, s32 map_height, s32 seed)
{
    f32 noise_scale = 4.0f / (f32)(map_width);
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            f32 nx = (f32)x * noise_scale;
            f32 ny = (f32)y * noise_scale;
            
            map_tile_grid[IDX2D(x, y, map_width)].elevation += SampleWarpedFBM(nx + 1.11f, ny + 2.788f, seed) * 0.7f;
           
            f32 temp_offset_elevation = 1.0f - map_tile_grid[IDX2D(x, y, map_width)].elevation;
            f32 equator = (f32)(map_height - 1) / 2.0f;
            f32 distance_from_equator = fabsf((f32)y - equator) / equator; // [0, 1]
            f32 temp_offset_latitude = 1.0f - distance_from_equator;
            map_tile_grid[IDX2D(x, y, map_width)].temperature = SampleWarpedFBM(nx + 3.422f, ny + 13.2f, seed) * 0.35f + temp_offset_elevation * 0.15f + temp_offset_latitude * 0.5f;
        
            nx *= 4.0f;
            ny *= 4.0f;
            // TODO: offset moisture based on adjacent plates
            //       more oceanic adjacent plates = higher base moisture and viceversa..
            map_tile_grid[IDX2D(x, y, map_width)].moisture = SampleWarpedFBM(nx + 5.56f, ny + 1.31f, seed);
        }
    }
}

void GenerateRiverFlows(tile_data *map_tile_grid, s32 map_width, s32 map_height)
{
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            // TODO: do not flow river in oceans
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
            if(accumulation > 1000) { map_tile_grid[IDX2D(x, y, map_width)].river_type = TILE_HAS_MAJOR_RIVER; }
            else if(accumulation > 500) { map_tile_grid[IDX2D(x, y, map_width)].river_type = TILE_HAS_STREAM; }
            else if(accumulation > 100) { map_tile_grid[IDX2D(x, y, map_width)].river_type = TILE_HAS_CREEK; }
            else { map_tile_grid[IDX2D(x, y, map_width)].river_type = TILE_HAS_NO_RIVER; }
        }
    }
}

int main(void)
{
    s32 seed = (s32)time(NULL);
    srand((u32)seed);

    s32 map_width = 1024; s32 map_height = 512;
    tile_data* map_tile_grid = (tile_data*)calloc((u32)(map_width * map_height), sizeof(tile_data));
    
    s32 plate_count = 150;
    plate_data *plates = (plate_data*)malloc(sizeof(plate_data) * plate_count);

    GeneratePlatesAndAssignBaseElevationToTiles(map_tile_grid, map_width, map_height, plates, plate_count);
    GenerateNoiseMap(map_tile_grid, map_width, map_height, seed);
    GenerateRiverFlows(map_tile_grid, map_width, map_height);

    u8* plates_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    u8* elevation_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    u8* moisture_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    u8* temperature_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            u32 plate_index = map_tile_grid[IDX2D(x, y, map_width)].closest_plate_index;
            plates_map[IDX2D(x, y, map_width)] = plates[plate_index].crust_type == CRUST_TYPE_OCEANIC ? 0u : 255u;
            elevation_map[IDX2D(x, y, map_width)] = (u8)(map_tile_grid[IDX2D(x, y, map_width)].elevation * 255.0f);
            moisture_map[IDX2D(x, y, map_width)] = (u8)(map_tile_grid[IDX2D(x, y, map_width)].moisture * 255.0f);
            temperature_map[IDX2D(x, y, map_width)] = (u8)(map_tile_grid[IDX2D(x, y, map_width)].temperature * 255.0f);
        }
    }
    ASSERT(stbi_write_png("output/plates.png", map_width, map_height, 1, plates_map, 1 * map_width));
    ASSERT(stbi_write_png("output/elevation.png", map_width, map_height, 1, elevation_map, 1 * map_width));
    ASSERT(stbi_write_png("output/moisture.png", map_width, map_height, 1, moisture_map, 1 * map_width));
    ASSERT(stbi_write_png("output/temperature.png", map_width, map_height, 1, temperature_map, 1 * map_width));

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

            f32 jitter_strength = 0.01f;
            moisture = moisture * 0.99f + stb_perlin_noise3_seed((f32)x * 10.0f + 2.5f, (f32)y * 10.0f + 4.5f, (f32)seed * 0.445f + 3.71f, 0, 0, 0, seed+222) * jitter_strength;
            temperature = temperature * 0.99f + stb_perlin_noise3_seed((f32)x * 10.0f + 2.134f, (f32)y * 10.0f + 13.14f, (f32)seed * 0.251f + 1.32f, 0, 0, 0, seed+333) * jitter_strength;

            biome_type_id biome = EvaluateBiome(elevation, moisture, temperature);
            
            color_rgb output_color = biome_color_table[biome];
            tile_has_river_type river_type = map_tile_grid[IDX2D(x, y, map_width)].river_type; 
            if(biome != BIOME_TYPE_SHALLOW_OCEAN && biome != BIOME_TYPE_OCEAN && biome != BIOME_TYPE_DEEP_OCEAN)
            {
                if(river_type == TILE_HAS_MAJOR_RIVER) { output_color = COLOR_DARK_BLUE; }
                else if(river_type == TILE_HAS_STREAM) { output_color = COLOR_BLUE; }
                else if(river_type == TILE_HAS_CREEK) { output_color = COLOR_BLUE_CREEK; }
            
                f32 equator = (f32)(map_height - 1) / 2.0f;
                f32 distance_from_equator = fabsf((f32)y - equator) / equator;
                if(distance_from_equator >= 0.95f)
                {
                    if(moisture <= 0.25f) { biome = BIOME_TYPE_ICE; }
                    else if(moisture <= 0.5f) { biome = BIOME_TYPE_SNOW; }
                    output_color = biome_color_table[biome];
                }
            }

            biome_stat_table[biome]++;
            output_image[IDX2D(x, y, map_width)] = output_color;
        }
    }
    ASSERT(stbi_write_png("output/map.png", map_width, map_height, 3, (u8*)output_image, 3 * map_width));
    
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