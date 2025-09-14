#include "tfss_read_write.h"

//#include <texture_filestorage_system.h>

#include <stb_image.h>
#include <zstd/zstd.h>

#include <stdio.h>

uint8_t paeth_predictor(uint8_t a, uint8_t b, uint8_t c) {
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);

    if (pa <= pb && pa <= pc) return a;
    if (pb <= pc) return b;
    return c;
}


// Filter Type 0 - None
void filter_none(uint8_t *filtered, const uint8_t *scanline, size_t length) {
    for (size_t i = 0; i < length; i++)
        filtered[i] = scanline[i];
}

// Filter Type 1 - Sub
void filter_sub(uint8_t *filtered, const uint8_t *scanline, size_t length, int bpp) 
{
    for (size_t i = 0; i < length; i++) {
        uint8_t left = (i >= bpp) ? scanline[i - bpp] : 0;
        filtered[i] = scanline[i] - left;
    }
}

// Filter Type 2 - Up
void filter_up(uint8_t *filtered, const uint8_t *scanline, const uint8_t *prev, size_t length) 
{
    for (size_t i = 0; i < length; i++) {
        uint8_t up = prev ? prev[i] : 0;
        filtered[i] = scanline[i] - up;
    }
}

// Filter Type 3 - Average
void filter_avg(uint8_t *filtered, const uint8_t *scanline, const uint8_t *prev, size_t length, int bpp) 
{
    for (size_t i = 0; i < length; i++) {
        uint8_t left = (i >= bpp) ? scanline[i - bpp] : 0;
        uint8_t up = prev ? prev[i] : 0;
        filtered[i] = scanline[i] - ((left + up) / 2);
    }
}

// Filter Type 4 - Paeth
void filter_paeth(uint8_t *filtered, const uint8_t *scanline, const uint8_t *prev, size_t length, int bpp) 
{
    for (size_t i = 0; i < length; i++) {
        uint8_t a = (i >= bpp) ? scanline[i - bpp] : 0;        // left
        uint8_t b = prev ? prev[i] : 0;                        // above
        uint8_t c = (i >= bpp && prev) ? prev[i - bpp] : 0;    // upper-left
        filtered[i] = scanline[i] - paeth_predictor(a, b, c);
    }
}

int select_best_filter(uint8_t *out, const uint8_t *scanline, const uint8_t *prev, size_t length, int bpp) 
{
    uint8_t* temp[5];
	for (int i = 0; i < 5; i++)
		temp[i] = malloc(length);

	if (!temp[0] || !temp[1] || !temp[2] || !temp[3] || !temp[4]) 
	{
		for (int i = 0; i < 5; i++) free(temp[i]);
		return -1;
	}
    int32_t sums[5] = {0};

    filter_none(temp[0], scanline, length);
    filter_sub(temp[1], scanline, length, bpp);
    filter_up(temp[2], scanline, prev, length);
    filter_avg(temp[3], scanline, prev, length, bpp);
    filter_paeth(temp[4], scanline, prev, length, bpp);

    for (int f = 0; f < 5; ++f) {
        for (size_t i = 0; i < length; ++i)
            sums[f] += abs((int)(temp[f][i]));
    }

    int best_filter = 0;
    int32_t best_score = sums[0];
    for (int f = 1; f < 5; ++f) {
        if (sums[f] < best_score) {
            best_score = sums[f];
            best_filter = f;
        }
    }

    // Write best filtered scanline
    out[0] = (uint8_t)best_filter;
    memcpy(out + 1, temp[best_filter], length);
	
	for (int i = 0; i < 5; i++) free(temp[i]);
    return best_filter;
}

int filter_data_raw(const uint8_t* in, uint8_t* out, uint32_t bpp, uint32_t wh[2])
{
    uint32_t width  = wh[0];
    uint32_t height = wh[1];
    size_t row_size = width * bpp;

    const uint8_t* prev = NULL;

    for (uint32_t y = 0; y < height; y++)
    {
		
        const uint8_t* scanline = in + y * row_size;
        uint8_t* filtered_row   = out + y * (row_size + 1);
		
		printf("%d %p %p\n", y, scanline, filtered_row);
		
        select_best_filter(filtered_row, scanline, prev, row_size, bpp);

        prev = scanline;
    }

    return 0;
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


// assumes images are placed side by side
// pedantically 0 and 1 are both just one layer 
int write_tfss(const char* name, char* data, uint32_t flags, uint32_t wh_dl[3], uint8_t format, uint8_t mip_count, uint8_t compression, uint8_t faces, mipmapper mipmap)
{
	FILE* file = fopen(name, "wb+");
	
	if(!file){fclose(file); return -1;};
	
	char magic[4] = {'T', 'F', 'S' , 'S'};
	fwrite(magic, sizeof(char), 4, file);
	
	fwrite(&flags, sizeof(uint32_t), 1, file);
	fwrite(wh_dl, sizeof(uint32_t), 3, file);
	

	fwrite(&format, sizeof(uint8_t), 1, file);
	fwrite(&compression, sizeof(uint8_t), 1, file);
	fwrite(&mip_count, sizeof(uint8_t), 1, file);
	fwrite(&faces, sizeof(uint8_t), 1, file);
	
	uint8_t bpp = bytes_per_pixel((tfss_formats)format);
	uint32_t width_height[2] = {wh_dl[0], wh_dl[1]};
		
	uint64_t zero[2] = {0, 0};
	
	if(wh_dl[2] > 2)
		for(int i = 0; i < wh_dl[2]; i++) fwrite(&zero, sizeof(uint32_t), 2, file);
	else fwrite(&zero, sizeof(uint32_t), 2, file);

	if (compression && format <= 14)
	{
		uint32_t filtered_size = (wh_dl[0] * bpp + 1) * wh_dl[1];

		uint64_t dest_size = ZSTD_compressBound(filtered_size);
		uint8_t* filtered = malloc(filtered_size);
		uint8_t* dest = malloc(dest_size);
		
		if (!filtered) 
		{ 
			fclose(file); 
			return -2; 
		}

		filter_data_raw(data, filtered, bpp, width_height);

		
		if (!dest) 
		{ 
			free(filtered); 
			fclose(file);
			return -3; 
		}
			
		if( wh_dl[2] < 2)
		{
			uint32_t size = (uint32_t)ZSTD_compress(dest, dest_size, filtered, filtered_size, compression);
			
			if (ZSTD_isError(size)) 
			{
				fprintf(stderr, "ZSTD compression error: %s\n", ZSTD_getErrorName(size));
				free(filtered);
				free(dest);
				fclose(file);
				return -4;
			}
			uint32_t offset = 0;
			fwrite(dest, sizeof(uint8_t), size, file);
			fseek(file, -size - sizeof(uint32_t) * 2, SEEK_CUR); // return to length
			fwrite(&offset, sizeof(uint32_t), 1, file);
			fwrite(&size, sizeof(uint32_t), 1, file);
			
			free(dest);
			free(filtered);
			fclose(file);
			return 0;
		}
		

		uint32_t true_size = wh_dl[0] * bpp * wh_dl[1];

		if (!filtered) 
		{ 
			fclose(file); 
			return -2; 
		}
		
		for(unsigned int i = 0; i < wh_dl[2]; i++)
		{

			filter_data_raw(data + true_size * i, filtered, bpp, width_height);
			
			uint64_t size = ZSTD_compress(dest, dest_size, filtered, filtered_size, compression);
			
			if (ZSTD_isError(size)) 
			{
				fprintf(stderr, "ZSTD compression error: %s\n", ZSTD_getErrorName(size));
				free(filtered);
				free(dest);
				fclose(file);
				return -4;
			}

			fwrite(dest, sizeof(uint8_t), size, file);
			
		}
		
		free(dest);
		free(filtered);
		fclose(file);
		return 0;
	}
	else if (format >= 14 && compression)
	{
		uint64_t true_size = bpp * width_height[0] * width_height[1];
		uint64_t dest_size = ZSTD_compressBound(true_size);
		uint8_t* dest = malloc(dest_size);
		for(unsigned int i = 0; i < wh_dl[2]; i++)
		{
			uint64_t size = ZSTD_compress(dest, dest_size, data + true_size * i, true_size, compression);
			if (ZSTD_isError(size)) 
			{
				fprintf(stderr, "ZSTD compression error: %s\n", ZSTD_getErrorName(size));
				free(dest);
				fclose(file);
				return -4;
			}
			
			fwrite(dest, sizeof(uint8_t), size, file);
		}
		
		free(dest);
		fclose(file);
		return 0;
	}
	else
	{
		fwrite(data, sizeof(uint8_t), bpp * width_height[0] * width_height[1], file);
		return 0;
	}
	
}