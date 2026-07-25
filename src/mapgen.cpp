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
color_rgb COLOR_ORANGE = ColorRGB(200, 160, 140);
color_rgb COLOR_BLUE = ColorRGB(50, 50, 255);
color_rgb biome_color_table[BIOME_TYPE_COUNT] = { COLOR_BLACK, COLOR_DARK_BLUE, COLOR_WHITE,
                                                  COLOR_GREEN, COLOR_YELLOW, COLOR_MUSTARD_GREEN,
                                                  COLOR_PURPLE, COLOR_DARK_GREEN, COLOR_LIGHT_BLUE,
                                                  COLOR_DARK_GREY, COLOR_ORANGE, COLOR_BLUE };
char *biome_name_table[BIOME_TYPE_COUNT] = { "The void", "Ocean", "Mountain", "Plain", "Desert", "Jungle", "Marsh", "Forest", "Tundra", "Highlands", "Savannah", "Shallow Ocean" };

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

biome_type_id EvaluateBiome(f32 elevation, f32 moisture, f32 temperature)
{
    if(elevation < 0.46f)
    {
        return(BIOME_TYPE_OCEAN);
    }
    if(elevation < 0.5f)
    {
        return(BIOME_TYPE_SHALLOW_OCEAN);
    }

    if(elevation > 0.95f)
    {
        return(BIOME_TYPE_MOUNTAIN);
    }

    if(elevation > 0.75f)
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
            if(moisture > 0.7f)
            {
                return(BIOME_TYPE_MARSH);
            }
            else if(moisture > 0.5f)
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

#include "float.h"

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

enum crust_type : u8
{
    CRUST_TYPE_CONTINENTAL = 0,
    CRUST_TYPE_OCEANIC,

    CRUST_TYPE_COUNT
};

enum plate_collision_type : u8
{
    PLATE_COLLISION_NONE = 0,
    PLATE_COLLISION_CONVERGENT,
    PLATE_COLLISION_DIVERGENT,
    PLATE_COLLISION_SLIDING,

    PLATE_COLLISION_COUNT
};

struct plate_data
{
    vec2i seed;
    direction drifting_direction;
    crust_type type;
};

s32 GetMinDistanceFromContinentalSeed(s32 ax, s32 ay, u8 tile_plate_index, plate_data *plates, s32 plate_count)
{
    if(plates[tile_plate_index].type == CRUST_TYPE_CONTINENTAL)
    {
        s32 result = ComputeDistanceInTiles(ax, ay, plates[tile_plate_index].seed);
        return(result);
    }

    s32 min_distance_from_land = INT_MAX;
    for(s32 idx = 0; idx < plate_count; idx++)
    {
        if(plates[idx].type == CRUST_TYPE_OCEANIC) { continue; }

        s32 dist = ComputeDistanceInTiles(ax, ay, plates[idx].seed);
        if(dist < min_distance_from_land)
        {
            min_distance_from_land = dist;
        }
    }
    return(min_distance_from_land);
}

struct tile_data
{
    f32 elevation;
    f32 moisture;

    u8 water_descent_direction; // TODO: should water fall towards multiple directions?
    u32 water_accumulation_count;

    u8 plate_index;
};

void GenerateNoiseMap(tile_data *map_tile_grid, s32 map_width, s32 map_height, s32 seed)
{
    f32 noise_scale = 16.0f / (f32)(map_width);
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            f32 nx = (f32)x * noise_scale;
            f32 ny = (f32)y * noise_scale;
            
            map_tile_grid[IDX2D(x, y, map_width)].elevation = SampleWarpedFBM(nx, ny, seed);;
            map_tile_grid[IDX2D(x, y, map_width)].moisture = SampleWarpedFBM(nx + 17.51f, ny + 31.82f, seed);;
        }
    }
}

// TODO: duplicate use of the word "seed" between noise/rng and plates assignement
void AssignAndApplyTectonicPlates(tile_data *map, s32 width, s32 height, plate_data *plates, s32 plate_count)
{
    for(s32 i = 0; i < plate_count; i++)
    {
        plates[i].seed.x = RandomInt(0, width - 1);
        plates[i].seed.y = RandomInt(0, height - 1);
        plates[i].drifting_direction = (direction)RandomInt(1, DIRECTION_COUNT - 1);
        plates[i].type = (crust_type)RandomInt(0, 1);
    }
    
    for(s32 x = 0; x < width; x++)
    {
        for(s32 y = 0; y < height; y++)
        {
            s32 closest_dist = INT_MAX;
            u8 closest_seed = 0;
            for(s32 i = 0; i < plate_count; i++)
            {
                s32 distance = ComputeDistanceInTiles(x, y, plates[i].seed);
                if(distance < closest_dist)
                {
                    closest_dist = distance;
                    closest_seed = (u8)i;
                }
            }
            map[IDX2D(x, y, width)].plate_index = closest_seed;
            if(plates[closest_seed].type == CRUST_TYPE_CONTINENTAL)
            {
                map[IDX2D(x, y, width)].elevation = (map[IDX2D(x, y, width)].elevation / 2.0f) + 0.5f;
            }
            else
            {
                // Oceanic plate, starts at 0.0f
            }

        }
    }
}

int main(void)
{
    s32 seed = (s32)time(NULL);
    srand((u32)seed);

    s32 map_width = 1024; s32 map_height = 512;
    f32 min_elevation = FLT_MAX/2.0f; f32 max_elevation = -FLT_MAX/2.0f;
    f32 min_moisture = FLT_MAX/2.0f; f32 max_moisture = -FLT_MAX/2.0f;
    s32 biome_stat_table[BIOME_TYPE_COUNT] = {};
    tile_data* map_tile_grid = (tile_data*)calloc((u32)(map_width * map_height), sizeof(tile_data));
    
    s32 plate_count = RandomInt(16, 24);
    plate_data *plates = (plate_data*)malloc(sizeof(plate_data) * plate_count);
    GenerateNoiseMap(map_tile_grid, map_width, map_height, seed);
    AssignAndApplyTectonicPlates(map_tile_grid, map_width, map_height, plates, plate_count);
    
    u8* collision_direction_map = (u8*)calloc((u32)map_width * map_height, sizeof(u8));
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            u8 plate_index = map_tile_grid[IDX2D(x, y, map_width)].plate_index;
            vec2i position = { x, y };
            s32 divergent_collision_count = 0;
            s32 convergent_collision_count = 0;
            s32 continental_convergent_count = 0;
            s32 oceanic_convergent_count = 0;
            s32 sliding_collision_count = 0;
            plate_collision_type final_collision_resolution = PLATE_COLLISION_NONE;
            for(s32 dir_idx = 1; dir_idx < DIRECTION_COUNT; dir_idx++)
            {
                vec2i other_position = position + adjacent_tile_from_direction[dir_idx];
                if(IN_BOUNDS2D(other_position.x, other_position.y, map_width, map_height))
                {
                    u8 other_plate_index = map_tile_grid[IDX2D(other_position.x, other_position.y, map_width)].plate_index;
                    if(plate_index != other_plate_index)
                    {
                        direction tile_drifting_direction = plates[plate_index].drifting_direction;
                        direction other_drifting_direction = plates[other_plate_index].drifting_direction;

                        vec2i direction_vector = adjacent_tile_from_direction[tile_drifting_direction];
                        vec2i other_direction_vector = adjacent_tile_from_direction[other_drifting_direction];

                        vec2i moved_position = position + direction_vector;
                        vec2i other_moved_position = other_position + other_direction_vector;

                        s32 distance = ComputeDistanceInTiles(moved_position.x, moved_position.y, other_moved_position);
                        if (distance == 0)
                        {
                            convergent_collision_count++;
                            crust_type other_crust_type = plates[other_plate_index].type;
                            if(other_crust_type == CRUST_TYPE_CONTINENTAL) { continental_convergent_count++; }
                            else { oceanic_convergent_count++; }
                        }
                        else if(distance == 1) { sliding_collision_count++; }
                        else if(distance >= 2) { divergent_collision_count++; }
                    }
                }
            }
            if(convergent_collision_count > sliding_collision_count)
            {
                if(convergent_collision_count > divergent_collision_count)
                {
                    final_collision_resolution = PLATE_COLLISION_CONVERGENT;
                }
                else
                {
                    final_collision_resolution = PLATE_COLLISION_DIVERGENT;
                }
            }
            else
            {
                if(sliding_collision_count > divergent_collision_count)
                {
                    final_collision_resolution = PLATE_COLLISION_SLIDING;
                }
                else
                {
                    final_collision_resolution = PLATE_COLLISION_DIVERGENT;
                }
            }
            
            f32 elevation_offset = 0.0f;
            if(final_collision_resolution == PLATE_COLLISION_DIVERGENT && divergent_collision_count != 0)
            {
                elevation_offset = -0.2f;
                collision_direction_map[IDX2D(x, y, map_width)] = 255u;
            }
            else if(final_collision_resolution == PLATE_COLLISION_SLIDING)
            {
                elevation_offset = 0.05f;
                collision_direction_map[IDX2D(x, y, map_width)] = 50u;
            }
            else if(final_collision_resolution == PLATE_COLLISION_CONVERGENT)
            {
                crust_type tile_crust_type = plates[plate_index].type;
                crust_type other_crust_type = continental_convergent_count >= oceanic_convergent_count ? CRUST_TYPE_CONTINENTAL : CRUST_TYPE_OCEANIC;
                if(tile_crust_type == CRUST_TYPE_CONTINENTAL && other_crust_type == CRUST_TYPE_CONTINENTAL)
                {
                    elevation_offset = 0.3f;
                }
                else if(tile_crust_type == CRUST_TYPE_CONTINENTAL && other_crust_type == CRUST_TYPE_OCEANIC)
                {
                    elevation_offset = 0.05f;
                }
                else if(tile_crust_type == CRUST_TYPE_OCEANIC && other_crust_type == CRUST_TYPE_CONTINENTAL)
                {
                    elevation_offset = -0.05f;
                }
                else if(tile_crust_type == CRUST_TYPE_OCEANIC && other_crust_type == CRUST_TYPE_OCEANIC)
                {
                    elevation_offset = 0.25f;
                }
                collision_direction_map[IDX2D(x, y, map_width)] = 160u;
            }
            map_tile_grid[IDX2D(x, y, map_width)].elevation += elevation_offset;
        }
    }

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

    u32 max_accumulation = 0;
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

                // TODO: add function so that this kind of indexing becomes map_tile_grid.At(current_pos) or something similar
                if(map_tile_grid[IDX2D(current_pos.x, current_pos.y, map_width)].water_accumulation_count > max_accumulation)
                {
                    max_accumulation = map_tile_grid[IDX2D(current_pos.x, current_pos.y, map_width)].water_accumulation_count;
                }
                current_dir = map_tile_grid[IDX2D(current_pos.x, current_pos.y, map_width)].water_descent_direction;
            }
        }
    }

    u8* accumulation_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    u8* plates_map = (u8*)malloc(sizeof(u8) * map_width * map_height);
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            f32 accumulation = (f32)map_tile_grid[IDX2D(x, y, map_width)].water_accumulation_count;
            f32 denom = (f32)max_accumulation;
            f32 value = accumulation / denom; // [0, 1]
            accumulation_map[IDX2D(x, y, map_width)] = (u8)(value * 255);

            s32 iter_count = 16;
            for(s32 i = 0; i < iter_count; i++)
            {
                map_tile_grid[IDX2D(x, y, map_width)].elevation -= (value * 0.1f);
            }

            u8 plate_index = map_tile_grid[IDX2D(x, y, map_width)].plate_index;
            crust_type type = plates[plate_index].type;
            plates_map[IDX2D(x, y, map_width)] = type == CRUST_TYPE_OCEANIC ? 0u : 255u;
        }
    }

    if(!stbi_write_png("output/accumulation.png", map_width, map_height, 1, accumulation_map, 1 * map_width))
    {
        printf("error with stbi_write_png.\n");
    }
    if(!stbi_write_png("output/plates.png", map_width, map_height, 1, plates_map, 1 * map_width))
    {
        printf("error with stbi_write_png.\n");
    }
    if(!stbi_write_png("output/collisions.png", map_width, map_height, 1, collision_direction_map, 1 * map_width))
    {
        printf("error with stbi_write_png.\n");
    }

    f32 max_distance_from_continental_seed = 0.0f;
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            u8 tile_plate_index = map_tile_grid[IDX2D(x, y, map_width)].plate_index;
            f32 dist = (f32)GetMinDistanceFromContinentalSeed(x, y, tile_plate_index, plates, plate_count);
            if(dist > max_distance_from_continental_seed)
            {
                max_distance_from_continental_seed = dist;
            }
        }
    }

    color_rgb* output_image = (color_rgb*)malloc(sizeof(color_rgb) * map_width * map_height);
    for(s32 x = 0; x < map_width; x++)
    {
        for(s32 y = 0; y < map_height; y++)
        {
            f32 elevation = map_tile_grid[IDX2D(x, y, map_width)].elevation;
            u8 tile_plate_index = map_tile_grid[IDX2D(x, y, map_width)].plate_index;
            f32 min_distance_from_land = (f32)GetMinDistanceFromContinentalSeed(x, y, tile_plate_index, plates, plate_count);

            // distance in [0, 1]
            f32 normalized_distance = min_distance_from_land / max_distance_from_continental_seed;
            ASSERT(normalized_distance >= 0.0f && normalized_distance <= 1.0f);

            f32 continentality = 1.0f - normalized_distance;
            f32 multiplier = expf(Clampf((continentality - 0.5f) / 2.0f, -0.5f, 0.1f));
            elevation = elevation * continentality * multiplier;
            elevation = Clampf(elevation, 0.0f, 1.0f);

            f32 moisture = map_tile_grid[IDX2D(x, y, map_width)].moisture;
            f32 equator = (f32)(map_height - 1) / 2.0f;
            f32 distance_from_equator = fabsf((f32)y - equator) / equator;
            f32 temperature = 1.0f - ((distance_from_equator + elevation) / 2.0f);
            biome_type_id biome = EvaluateBiome(elevation, moisture, temperature);

            biome_stat_table[biome]++;
            output_image[IDX2D(x, y, map_width)] = biome_color_table[biome];
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