#include "stdio.h"

#include "math.h"
#include "stdlib.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(push, 0)
#include "stb_image_write.h"
#pragma warning(pop)

#include "nom.h"

enum biome_type_id : u16
{
    BIOME_TYPE_NULL = 0,
    BIOME_TYPE_OCEAN,

    BIOME_TYPE_COUNT,
};

struct tile_data
{
    biome_type_id biome_id;
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
color_rgb COLOR_BLUE = ColorRGB(0, 0, 255);

int main(void)
{
    color_rgb biome_color_table[BIOME_TYPE_COUNT] = { COLOR_BLACK, COLOR_BLUE };

    s32 map_width = 64;
    s32 map_height = 64;
    tile_data* map_tile_grid = (tile_data*)malloc(sizeof(tile_data) * map_width * map_height);
    for(s32 i = 0; i < map_width; i++)
    {
        for(s32 j = 0; j < map_height; j++)
        {
            map_tile_grid[IDX2D(i, j, map_width)] = {};
            map_tile_grid[IDX2D(i, j, map_width)].biome_id = BIOME_TYPE_OCEAN;
        }
    }

    color_rgb* output_image = (color_rgb*)malloc(sizeof(color_rgb) * map_width * map_height);
    for(s32 i = 0; i < map_width; i++)
    {
        for(s32 j = 0; j < map_height; j++)
        {
            output_image[IDX2D(i, j, map_width)] = biome_color_table[map_tile_grid[IDX2D(i, j, map_width)].biome_id];
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