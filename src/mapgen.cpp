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
color_rgb COLOR_GREEN = ColorRGB(100, 220, 100);
color_rgb COLOR_MUSTARD_GREEN = ColorRGB(110, 110, 48);
color_rgb COLOR_YELLOW = ColorRGB(255, 255, 100);
color_rgb COLOR_PURPLE = ColorRGB(127, 0, 127);
color_rgb COLOR_GREY = ColorRGB(127, 127, 127);
color_rgb COLOR_ORANGE = ColorRGB(200, 160, 140);
color_rgb COLOR_BROWN = ColorRGB(120, 65, 65);
color_rgb biome_color_table[BIOME_TYPE_COUNT] = { COLOR_BLACK, COLOR_VERY_DARK_BLUE, COLOR_DARK_BLUE, COLOR_BLUE,
                                                  COLOR_GREY, COLOR_GREEN, COLOR_YELLOW, COLOR_MUSTARD_GREEN,
                                                  COLOR_PURPLE, COLOR_DARK_GREEN, COLOR_LIGHT_BLUE,
                                                  COLOR_ORANGE, COLOR_WHITE, COLOR_AQUA, COLOR_BROWN,
                                                  COLOR_LIGHT_AQUA };
char *biome_name_table[BIOME_TYPE_COUNT] = { "The void", "Deep Ocean", "Ocean", "Shallow Ocean", "Mountain", "Plain", "Desert", "Jungle", "Marsh",
                                             "Forest", "Tundra", "Savannah", "Snow", "Rainforest", "Taiga", "Ice" };

f32 biome_vegetation_base_table[BIOME_TYPE_COUNT] = { 0.0f, 0.0f, 0.0f, 0.1f, 0.2f, 0.7f, 0.05f, 0.9f, 0.8f,
                                             0.85f, 0.1f, 0.5f, 0.0f, 1.0f, 0.6f, 0.0f };

f32 biome_vegetation_regen_table[BIOME_TYPE_COUNT] = { 0.0f, 0.0f, 0.0f, 0.01f, 0.02f, 0.1f, 0.01f, 0.2f, 0.2f,
                                             0.15f, 0.02f, 0.05f, 0.0f, 0.2f, 0.1f, 0.0f };


biome_type_id EvaluateBiome(f32 elevation, f32 moisture, f32 temperature)
{
    if(elevation < 0.35f) { return(BIOME_TYPE_DEEP_OCEAN); }
    if(elevation < 0.475f) { return(BIOME_TYPE_OCEAN); }
    if(elevation < 0.5f) { return(BIOME_TYPE_SHALLOW_OCEAN); }
    if(elevation > 0.8f || temperature <= 0.25f)
    {
        if(moisture > 0.45f) return(BIOME_TYPE_SNOW);
        else return(BIOME_TYPE_ICE);
    }
    if(elevation > 0.75f) { return(BIOME_TYPE_MOUNTAIN); }

    if(temperature >= 0.65f)
    {
        if(moisture >= 0.6f) { return(BIOME_TYPE_JUNGLE); }
        else if(moisture >= 0.4f) { return(BIOME_TYPE_SAVANNAH); }
        else { return(BIOME_TYPE_DESERT); }
    }
    else if(temperature >= 0.425f)
    {
        if(moisture >= 0.65f) { return(BIOME_TYPE_RAINFOREST); }
        else if(moisture >= 0.5f) { return(BIOME_TYPE_FOREST); }
        else { return(BIOME_TYPE_PLAIN); }
    }
    else
    {
        if(moisture >= 0.65f) { return(BIOME_TYPE_MARSH); }
        else if(moisture >= 0.45f) { return(BIOME_TYPE_TAIGA); }
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

struct nation_data
{
    u8 tech_level;
    u32 total_population;
};

struct tile_data
{    
    u32 closest_plate_index;
    biome_type_id biome;

    f32 elevation;
    f32 moisture;
    f32 temperature;

    u8 water_descent_direction;
    s32 water_accumulation_count;
    tile_has_river_type river_type;

    u8 distance_from_water;
    u8 distance_from_river;

    f32 vegetation_density;
    f32 vegetation_capacity;
    f32 vegetation_regeneration_rate;

    f32 food_stock;
    u32 owner_nation_index;
    u32 population;
};

struct tile_map
{
    s32 width;
    s32 height;
    tile_data *tiles;
    s32 biome_stat_table[BIOME_TYPE_COUNT];
    f32 ocean_percentage;

    plate_data *plates;
    s32 plate_count;

    nation_data *nations;
    s32 nation_count;
    s32 nation_capacity;

    u32 total_population;

    tile_data *GetTile(s32 x, s32 y) { return(tiles + (y * width + x)); }
    tile_data *GetTile(vec2i p) { return(tiles + (p.y * width + p.x)); }
    plate_data *GetPlate(s32 i) { return(plates + i); }
    plate_data *GetPlate(u32 i) { return(plates + i); }
};

void GeneratePlatesAndAssignBaseElevationToTiles(tile_map map)
{
    for(s32 i = 0; i < map.plate_count; i++)
    {
        s32 roll = RandomInt(1, 4);
        u8 crust_type = roll == 1 ? CRUST_TYPE_CONTINENTAL : CRUST_TYPE_OCEANIC;
        map.GetPlate(i)->crust_type = crust_type;
        map.GetPlate(i)->base_elevation = crust_type == CRUST_TYPE_OCEANIC ? 0.0f : 0.3f;
        map.GetPlate(i)->center = { RandomInt(4, map.width - 4), RandomInt(4, map.height - 4) };
    }

    for(s32 x = 0; x < map.width; x++)
    {
        for(s32 y = 0; y < map.height; y++)
        {
            s32 closest_dist[3] = {INT_MAX, INT_MAX, INT_MAX};
            u32 closest_plates[3] = {0, 1, 2};
            for(s32 i = 0; i < map.plate_count; i++)
            {
                s32 distance = ComputeDistanceInTiles(x, y, map.GetPlate(i)->center);
                
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
            map.GetTile(x, y)->closest_plate_index = closest_plates[0];

            plate_data plate0 = *(map.GetPlate(closest_plates[0]));
            plate_data plate1 = *(map.GetPlate(closest_plates[1]));
            plate_data plate2 = *(map.GetPlate(closest_plates[2]));
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
            map.GetTile(x, y)->elevation = elevation;
        }
    }
}

void GenerateNoiseMap(tile_map map, s32 seed)
{
    f32 noise_scale_large = GetNoiseScaleForLargeDetails(map.width, map.height);
    f32 noise_scale_medium = GetNoiseScaleForMediumDetails(map.width, map.height);
    for(s32 x = 0; x < map.width; x++)
    {
        for(s32 y = 0; y < map.height; y++)
        {
            f32 nx = (f32)x * noise_scale_large;
            f32 ny = (f32)y * noise_scale_large;
            
            map.GetTile(x, y)->elevation += SampleWarpedFBM(nx, ny, seed) * 0.7f;
           
            f32 temp_offset_elevation = 1.0f - map.GetTile(x, y)->elevation;
            f32 equator = (f32)(map.height - 1) / 2.0f;
            f32 distance_from_equator = fabsf((f32)y - equator) / equator; // [0, 1]
            f32 temp_offset_latitude = 1.0f - distance_from_equator;
            map.GetTile(x, y)->temperature = SampleWarpedFBM(nx + PHI32, ny + PHI32, seed) * 0.3f +
                                                                    temp_offset_elevation * 0.2f +
                                                                    temp_offset_latitude * 0.5f;

            nx = (f32)x * noise_scale_medium;
            ny = (f32)y * noise_scale_medium;
            map.GetTile(x, y)->moisture = SampleWarpedFBM(nx - PHI32, ny - PHI32, seed);
        }
    }
}

void GenerateRiverFlows(tile_map map)
{
    for(s32 x = 0; x < map.width; x++)
    {
        for(s32 y = 0; y < map.height; y++)
        {
            f32 elev = map.GetTile(x, y)->elevation;
            if(elev < 0.5f) { continue; } // NOTE: skip oceans

            u8 steepest_dir = DIRECTION_NONE;
            f32 max_diff = 0.0f;
            for(u8 dir = 0; dir < DIRECTION_COUNT; dir++)
            {
                vec2i pos = {x, y};
                vec2i p_adjacent = pos + adjacent_tile_from_direction[dir];
                if(IN_BOUNDS2D(p_adjacent.x, p_adjacent.y, map.width, map.height))
                {
                    f32 adj_elev = map.GetTile(p_adjacent.x, p_adjacent.y)->elevation;
                    f32 diff = elev - adj_elev;
                    if(diff > max_diff)
                    {
                        max_diff = diff;
                        steepest_dir = dir;
                    }
                } 
            }
            map.GetTile(x, y)->water_descent_direction = steepest_dir;
        }
    }

    for(s32 x = 0; x < map.width; x++)
    {
        for(s32 y = 0; y < map.height; y++)
        {
            vec2i current_pos = { x, y };
            u8 current_dir = map.GetTile(x, y)->water_descent_direction;
            while(current_dir != DIRECTION_NONE)
            { 
                vec2i movement = adjacent_tile_from_direction[current_dir];
                current_pos = current_pos + movement;
                ASSERT(IN_BOUNDS2D(current_pos.x, current_pos.y, map.width, map.height));
                
                map.GetTile(current_pos.x, current_pos.y)->water_accumulation_count += 1;
                current_dir = map.GetTile(current_pos.x, current_pos.y)->water_descent_direction;
            }
        }
    }

    for(s32 x = 0; x < map.width; x++)
    {
        for(s32 y = 0; y < map.height; y++)
        {
            s32 accumulation = map.GetTile(x, y)->water_accumulation_count;
            if(accumulation > 1000) { map.GetTile(x, y)->river_type = TILE_HAS_MAJOR_RIVER; }
            else if(accumulation > 400) { map.GetTile(x, y)->river_type = TILE_HAS_STREAM; }
            else if(accumulation > 100) { map.GetTile(x, y)->river_type = TILE_HAS_CREEK; }
            else { map.GetTile(x, y)->river_type = TILE_HAS_NO_RIVER; }
        }
    }
}

inline b32 IsOcean(biome_type_id biome)
{
    if(biome == BIOME_TYPE_DEEP_OCEAN || biome == BIOME_TYPE_OCEAN || biome == BIOME_TYPE_SHALLOW_OCEAN)
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

inline b32 HasRiver(tile_data *tile)
{
    if(tile->river_type != TILE_HAS_NO_RIVER)
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

inline b32 HasWaterSource(tile_data *tile)
{
    if(IsOcean(tile->biome) || tile->river_type != TILE_HAS_NO_RIVER)
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

void AssignBiomes(tile_map *map, s32 seed)
{
    for(s32 x = 0; x < map->width; x++)
    {
        for(s32 y = 0; y < map->height; y++)
        {
            tile_data *tile = map->GetTile(x, y);
            f32 elevation = Clampf(tile->elevation, 0.0f, 1.0f);
            f32 moisture = Clampf(tile->moisture, 0.0f, 1.0f);
            f32 temperature = Clampf(tile->temperature, 0.0f, 1.0f);

            f32 jitter_strength = 0.01f;
            moisture = moisture * 0.99f + Perlin2D((f32)x, (f32)y, seed) * jitter_strength;
            temperature = temperature * 0.99f + Perlin2D((f32)x + PHI32, (f32)y + PHI32, seed) * jitter_strength;

            tile->biome = EvaluateBiome(elevation, moisture, temperature);
            map->biome_stat_table[tile->biome]++;

            f32 vegetation_capacity = biome_vegetation_base_table[tile->biome];
            vegetation_capacity *= Clampf(moisture * 1.2f, 0.05f, 1.0f);
            vegetation_capacity *= expf(-powf((temperature - 0.5f) / 0.25f, 2.0f));
            if(tile->river_type == TILE_HAS_MAJOR_RIVER) { vegetation_capacity *= 1.5f; }
            tile->vegetation_capacity = Clampf(vegetation_capacity, 0.0f, 1.0f);
            tile->vegetation_density = tile->vegetation_capacity * 0.8f;
            tile->vegetation_regeneration_rate = biome_vegetation_regen_table[tile->biome];

            if(HasWaterSource(tile)) { tile->distance_from_water = 0; }
            else { tile->distance_from_water = 255u; }

            if(HasRiver(tile)) { tile->distance_from_river = 0; }
            else { tile->distance_from_river = 255u; }
        }
    }
    for(u8 radius = 0; radius < 32; radius++)
    {
        for(s32 x = 1; x < map->width - 1; x++)
        {
            for(s32 y = 1; y < map->height - 1; y++)
            {
                tile_data *tile = map->GetTile(x, y);
                if(tile->distance_from_water == radius)
                {
                    for(s32 dx = -1; dx <= 1; dx++)
                    {
                        for(s32 dy = -1; dy <= 1; dy++)
                        {
                            vec2i other_tile = { x + dx, y + dy };
                            ASSERT(IN_BOUNDS2D(other_tile.x, other_tile.y, map->width, map->height));
                            map->GetTile(other_tile)->distance_from_water = MIN(map->GetTile(other_tile)->distance_from_water, radius + 1u);
                        }
                    }
                }
                if(tile->distance_from_river == radius)
                {
                    for(s32 dx = -1; dx <= 1; dx++)
                    {
                        for(s32 dy = -1; dy <= 1; dy++)
                        {
                            vec2i other_tile = { x + dx, y + dy };
                            ASSERT(IN_BOUNDS2D(other_tile.x, other_tile.y, map->width, map->height));
                            map->GetTile(other_tile)->distance_from_river = MIN(map->GetTile(other_tile)->distance_from_river, radius + 1u);
                        }
                    }
                }
                
            }
        }
    }
    s32 total_ocean_count = map->biome_stat_table[BIOME_TYPE_DEEP_OCEAN] + map->biome_stat_table[BIOME_TYPE_OCEAN] + map->biome_stat_table[BIOME_TYPE_SHALLOW_OCEAN];
    f32 total_ocean_percentage = (f32)total_ocean_count / (f32)(map->width * map->height);
    if(total_ocean_percentage > 0.8f || total_ocean_percentage < 0.55f)
    {
        printf("\n\nOceans too big or too small (%.1f%%), try regerating the map.\n", total_ocean_percentage);
    }
    map->ocean_percentage = total_ocean_percentage;
}

void SpawnSettelements(tile_map *map)
{
    for(s32 x = 0; x < map->width; x++)
    {
        for(s32 y = 0; y < map->height; y++)
        {
            tile_data *tile = map->GetTile(x, y);
            if(IsOcean(tile->biome) || tile->biome == BIOME_TYPE_ICE || tile->biome == BIOME_TYPE_SNOW) { continue; }
            if(tile->temperature <= 0.3f || tile->temperature >= 0.7f) { continue; }
            if(tile->moisture <= 0.3f) { continue; }
            
            f32 multiplier = 2.0f / (f32)(tile->distance_from_river + 1);
            multiplier += 1.0f / (f32)(tile->distance_from_water*tile->distance_from_water + 1);
            if(tile->biome == BIOME_TYPE_PLAIN || tile->biome == BIOME_TYPE_FOREST || tile->biome == BIOME_TYPE_RAINFOREST) { multiplier += 1.0f; }
            multiplier += tile->vegetation_density / 2.0f;
            
            tile->population = (u32)(multiplier * 120.0f);
            map->total_population += tile->population;
        }
    }
}

tile_map CreateMap(s32 width, s32 height, s32 seed)
{
    tile_map map = {};
    map.width = width;
    map.height = height;
    map.tiles = (tile_data*)calloc((u32)(width * height), sizeof(tile_data));
    memset(&map.biome_stat_table, 0, sizeof(map.biome_stat_table));
    map.plate_count = 150;
    map.plates = (plate_data*)malloc(sizeof(plate_data) * map.plate_count);
    map.nation_capacity = 256;
    map.nation_count = 0;
    map.nations = (nation_data*)malloc(sizeof(nation_data) * map.nation_capacity);
    map.total_population = 0u;

    GeneratePlatesAndAssignBaseElevationToTiles(map);
    GenerateNoiseMap(map, seed);
    GenerateRiverFlows(map);
    AssignBiomes(&map, seed);
    SpawnSettelements(&map);
    
    return(map);
}

void OutputMap(tile_map map, const char* filename_prefix)
{
    // TODO: create different snapshot files based on time (start, end, etc..)
    u8* plates_map = (u8*)malloc(sizeof(u8) * map.width * map.height);
    u8* elevation_map = (u8*)malloc(sizeof(u8) * map.width * map.height);
    u8* moisture_map = (u8*)malloc(sizeof(u8) * map.width * map.height);
    u8* temperature_map = (u8*)malloc(sizeof(u8) * map.width * map.height);
    u8* vegetation_map = (u8*)malloc(sizeof(u8) * map.width * map.height);
    u8* water_map = (u8*)malloc(sizeof(u8) * map.width * map.height);
    u8* population_map = (u8*)malloc(sizeof(u8) * map.width * map.height);
    for(s32 x = 0; x < map.width; x++)
    {
        for(s32 y = 0; y < map.height; y++)
        {
            tile_data *tile = map.GetTile(x, y);
            u32 plate_index = tile->closest_plate_index;
            plates_map[y * map.width + x] = map.GetPlate(plate_index)->crust_type == CRUST_TYPE_OCEANIC ? 0u : 255u;
            elevation_map[y * map.width + x] = (u8)(tile->elevation * 255.0f);
            moisture_map[y * map.width + x] = (u8)(tile->moisture * 255.0f);
            temperature_map[y * map.width + x] = (u8)(tile->temperature * 255.0f);
            vegetation_map[y * map.width + x] = (u8)(tile->vegetation_density * 255.0f);
            water_map[y * map.width + x] = (u8)(tile->distance_from_water);
            population_map[y * map.width + x] = (u8)(Clampf((f32)tile->population, 0.0f, 255.0f));
        }
    }
    ASSERT(stbi_write_png("output/plates.png", map.width, map.height, 1, plates_map, 1 * map.width));
    ASSERT(stbi_write_png("output/elevation.png", map.width, map.height, 1, elevation_map, 1 * map.width));
    ASSERT(stbi_write_png("output/moisture.png", map.width, map.height, 1, moisture_map, 1 * map.width));
    ASSERT(stbi_write_png("output/temperature.png", map.width, map.height, 1, temperature_map, 1 * map.width));
    ASSERT(stbi_write_png("output/vegetation.png", map.width, map.height, 1, vegetation_map, 1 * map.width));
    ASSERT(stbi_write_png("output/water.png", map.width, map.height, 1, water_map, 1 * map.width));
    ASSERT(stbi_write_png("output/population.png", map.width, map.height, 1, population_map, 1 * map.width));

    color_rgb* output_image = (color_rgb*)malloc(sizeof(color_rgb) * map.width * map.height);
    for(s32 x = 0; x < map.width; x++)
    {
        for(s32 y = 0; y < map.height; y++)
        {
            biome_type_id biome = map.GetTile(x, y)->biome;
            color_rgb output_color = biome_color_table[biome];
            tile_has_river_type river_type = map.GetTile(x, y)->river_type;
            if(river_type == TILE_HAS_MAJOR_RIVER) { output_color = COLOR_DARK_BLUE; }
            else if(river_type == TILE_HAS_STREAM) { output_color = COLOR_BLUE; }
            else if(river_type == TILE_HAS_CREEK) { output_color = COLOR_BLUE_CREEK; }

            output_image[y * map.width + x] = output_color;
        }
    }
    ASSERT(stbi_write_png("output/map.png", map.width, map.height, 3, (u8*)output_image, 3 * map.width));
    
    printf("\n========= MAP =========\n");
    for(s32 i = 0; i < BIOME_TYPE_COUNT; i++)
    {
        printf("%s: %d (%.1f%%).  ", biome_name_table[i], map.biome_stat_table[i], (f32)map.biome_stat_table[i] * 100.0f / (f32)(map.width * map.height));
        PrintLineEveryN(i, 3);
    }
}

void SimulateMap(tile_map *map)
{
    for(s32 x = 0; x < map->width; x++)
    {
        for(s32 y = 0; y < map->height; y++)
        {
            tile_data *tile = map->GetTile(x, y);
            tile->vegetation_density += (tile->vegetation_capacity - tile->vegetation_density) * tile->vegetation_regeneration_rate; // TODO: vary based on season
            tile->vegetation_density = Clampf(tile->vegetation_density, 0.0f, tile->vegetation_capacity);
        }
    }
}

int main(void)
{
    s32 seed = (s32)time(NULL);
    srand((u32)seed);
    printf("Seed: %d.\n", seed);
    
    s32 map_width = 1024; s32 map_height = 512;
    tile_map map = CreateMap(map_width, map_height, seed);
    OutputMap(map, "start");
    printf("\nTotal population: %u.\n", map.total_population);
    u32 total_months = 10 * 12;
    u32 ticks_per_month = 1;
    u32 tick_count = total_months * ticks_per_month;
    for(u32 i = 0; i < tick_count; i++)
    {
        SimulateMap(&map);
    }
    OutputMap(map, "end");

    return(0);
}