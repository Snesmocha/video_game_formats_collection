#include "tfss_read_write.h"

//#include <texture_filestorage_system.h>

#include <stdio.h>

#include <stb_image.h>
#include <zstd/zstd.h>


void unfilter_none(uint8_t *out, const uint8_t *filtered, size_t length) {
    memcpy(out, filtered, length);
}

void unfilter_sub(uint8_t *out, const uint8_t *filtered, size_t length, int bpp) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t left = (i >= bpp) ? out[i - bpp] : 0;
        out[i] = filtered[i] + left;
    }
}

void unfilter_up(uint8_t *out, const uint8_t *filtered, const uint8_t *prev, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t up = prev ? prev[i] : 0;
        out[i] = filtered[i] + up;
    }
}

void unfilter_avg(uint8_t *out, const uint8_t *filtered, const uint8_t *prev, size_t length, int bpp) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t left = (i >= bpp) ? out[i - bpp] : 0;
        uint8_t up = prev ? prev[i] : 0;
        out[i] = filtered[i] + ((left + up) / 2);
    }
}

void unfilter_paeth(uint8_t *out, const uint8_t *filtered, const uint8_t *prev, size_t length, int bpp) {
    for (size_t i = 0; i < length; ++i) {
        uint8_t a = (i >= bpp) ? out[i - bpp] : 0;
        uint8_t b = prev ? prev[i] : 0;
        uint8_t c = (i >= bpp && prev) ? prev[i - bpp] : 0;
        out[i] = filtered[i] + paeth_predictor(a, b, c);
    }
}

void decode_scanline(uint8_t *out, const uint8_t *in, const uint8_t *prev, size_t length, int bpp) {
    uint8_t type = in[0];
    const uint8_t *filtered = in + 1;
    switch (type) {
        case 0: unfilter_none(out, filtered, length); break;
        case 1: unfilter_sub(out, filtered, length, bpp); break;
        case 2: unfilter_up(out, filtered, prev, length); break;
        case 3: unfilter_avg(out, filtered, prev, length, bpp); break;
        case 4: unfilter_paeth(out, filtered, prev, length, bpp); break;
        default: fprintf(stderr, "Unknown filter type: %d\n", type); break;
    }
}

void unfilter_data_raw(uint8_t *out, const uint8_t *compressed_filtered_data, int width, int height, int bpp) {
    size_t row_length = width * bpp;
    const uint8_t *in_row = compressed_filtered_data;
    uint8_t *prev_row = NULL;

    for (int y = 0; y < height; y++) 
	{
        uint8_t *out_row = out + y * row_length;
        decode_scanline(out_row, in_row, prev_row, row_length, bpp);
        prev_row = out_row;
        in_row += 1 + row_length; // 1 filter byte + row bytes
    }
}

static inline size_t bytes_per_pixel(tfss_formats fmt)
{
    switch (fmt)
    {
        case TF_R8:           return 1;
        case TF_RG8:          return 2;
        case TF_RGB233:       return 1; // 2+3+3 = 8 bits packed
        case TF_RGB332:       return 1; // 3+3+2 = 8 bits packed
        case TF_RGB8:         return 3;
        case TF_RGB565:       return 2; // 16 bits
        case TF_RGBA8:        return 4;
        case TF_RGBA4444:     return 2; // 16 bits
        case TF_RGBA5551:     return 2; // 16 bits
        case TF_RGBA1010102:  return 4; // 32 bits
        case TF_R16F:         return 2; // 16-bit float
        case TF_RG16F:        return 4; // 16-bit float * 2
        case TF_RGB16F:       return 6; // 16-bit float * 3
        case TF_RGBA16F:      return 8; // 16-bit float * 4
        case TF_RGBA16:       return 8; // 16-bit integer * 4
        case TF_BCN1:         return 0; // compressed, block-based
        case TF_BCN2:         return 0; 
        case TF_BCN3:         return 0; 
        case TF_BCN5:         return 0; 
        case TF_BCN6N:        return 0;
        case TF_BCN7:         return 0;
        case TF_ETC1:         return 0;
        case TF_ETC2:         return 0;
        case TF_ETC4:         return 0;
        case TF_ASTC:         return 0; // compressed
        default:              return 0;
    }
}



int read_tfss(const char* name, char* out, uint32_t* flags, uint32_t wh_dl[3], uint8_t* format, uint8_t* mip_count, uint8_t* faces)
{
	FILE* file = fopen(name, "rb+");
	
	if(!file){fclose(file); return -1;};
	
	char magic[4];
	fread(magic, sizeof(char), 4, file);
	
	fread(flags, sizeof(uint32_t), 1, file);
	fread(wh_dl, sizeof(uint32_t), 3, file);
	
	uint8_t compression = 1;
	
	fread(format, sizeof(uint8_t), 1, file);
	fread(&compression, sizeof(uint8_t), 1, file);
	fread(mip_count, sizeof(uint8_t), 1, file);
	fread(faces, sizeof(uint8_t), 1, file);
	
	uint8_t bpp = bytes_per_pixel((tfss_formats)*format);
	uint32_t wh = wh_dl[0] * wh_dl[1];
	char* dest = malloc(bpp * wh);
	
	uint32_t lookup_map_size = 0;
	uint32_t compressed_size = 0, offset = 0;
	
	if (compression && *format <= 14)
	{
		if( wh_dl[2] == 0)
		{
			if(!(*mip_count))
			{
				lookup_map_size = *mip_count;
				fread(&offset, sizeof(uint32_t), 1, file);
				fread(&compressed_size, sizeof(uint32_t), 1, file);
				char* compressed = malloc(compressed_size);
				fread(compressed, sizeof(char), compressed_size, file);
				
				uint64_t decompressed_size =  (wh_dl[0]* bpp + 1) * wh_dl[1];

				char* decompressed = malloc(decompressed_size);
				size_t result = ZSTD_decompress(decompressed, decompressed_size, compressed, compressed_size);
				if (ZSTD_isError(result))
				{
                    free(compressed);
                    free(decompressed);
                    fclose(file);
                    return -1;
                }
				
				unfilter_data_raw(out, decompressed, wh_dl[0], wh_dl[1], bpp);
				
				free(compressed);
				free(decompressed);
				fclose(file);
				
				return 0;
			}
			
			
			return 0;
		}
	}
	else if (*format >= 14 && compression)
	{	
	
	}
	else
	{
		
	}
}


/*
TFSS load_tfss_header(const char* name)
{
    TFSS header = {0};
    FILE* file = fopen(name, "rb");
   
    char magic[4] = {0};
    fread(magic, sizeof(char), 4, file);

    uint32_t size = 0;
    fread(&size, sizeof(uint32_t), 4, file);

    for(int i = 0; i < size; i++)
    {
	
        uint32_t width;            // Texture width
        uint32_t height;           // Texture height
        uint32_t depth;            // 3D texture depth (optional, 0 if 2D)
        
        uint8_t mip_count;         // Number of mipmap levels
        uint8_t array_size;        // Number of layers (for texture arrays), if set to zero then it is only a single texture
        uint8_t image_format;       // Texture format (e.g., RGBA8, BC7)
        uint8_t color_format;      //  color format so number of colors

        uint32_t texture_size;             // Total size of the texture data in bytes

        fread(&width,  sizeof(uint32_t), 4, file);
        fread(&height, sizeof(uint32_t), 4, file);
        fread(&depth,  sizeof(uint32_t), 4, file);
        
        fread(&mip_count, sizeof(uint8_t), 4, file);
        fread(&array_size, sizeof(uint8_t), 4, file);
        fread(&image_format, sizeof(uint8_t), 4, file);
        fread(&color_format, sizeof(uint8_t), 4, file);

        fread(&texture_size,  sizeof(uint32_t), 4, file);

        // set file header here

        fseek(file, size, SEEK_CUR);
    }

    fclose(file);
    
    return header;
}

TFSS load_tfss_header_index(const char* name, int position)
{
    TFSS header = {0};
    FILE* file = fopen(name, "rb");
   
    char magic[4] = {0};
    fread(magic, sizeof(char), 4, file);

    uint32_t size = 0;
    fread(&size, sizeof(uint32_t), 4, file);


    for(int i = 0; i < position; i++)
    {
        fseek(file, 48, SEEK_CUR);
        uint32_t temp_read = 0;
        fread(&temp_read,  sizeof(uint32_t), 4, file);
        fseek(file, temp_read, SEEK_CUR);
    }
    

    uint32_t width;            // Texture width
    uint32_t height;           // Texture height
    uint32_t depth;            // 3D texture depth (optional, 0 if 2D)
    
    uint8_t mip_count;         // Number of mipmap levels
    uint8_t array_size;        // Number of layers (for texture arrays), if set to zero then it is only a single texture
    uint8_t image_format;       // Texture format (e.g., RGBA8, BC7)
    uint8_t color_format;      //  color format so number of colors

    uint32_t texture_size;             // Total size of the texture data in bytes

    fread(&width,  sizeof(uint32_t), 4, file);
    fread(&height, sizeof(uint32_t), 4, file);
    fread(&depth,  sizeof(uint32_t), 4, file);
    
    fread(&mip_count, sizeof(uint8_t), 4, file);
    fread(&array_size, sizeof(uint8_t), 4, file);
    fread(&image_format, sizeof(uint8_t), 4, file);
    fread(&color_format, sizeof(uint8_t), 4, file);

    fread(&texture_size,  sizeof(uint32_t), 4, file);

    fclose(file);
    
    return header;
}

void load_tfss_index(const char* name, uint8_t* data, int* bytes_per_pixel, int* width, int* height, int index)
{
	
}

*/
