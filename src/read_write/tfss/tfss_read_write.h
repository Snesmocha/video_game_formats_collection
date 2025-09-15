#ifndef TFSS_FILE_CREATION_H
#define TFSS_FILE_CREATION_H


#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t paeth_predictor(uint8_t a, uint8_t b, uint8_t c);

void unfilter_none(uint8_t *out, const uint8_t *filtered, size_t length);
void unfilter_sub(uint8_t *out, const uint8_t *filtered, size_t length, int bpp);
void unfilter_up(uint8_t *out, const uint8_t *filtered, const uint8_t *prev, size_t length);
void unfilter_avg(uint8_t *out, const uint8_t *filtered, const uint8_t *prev, size_t length, int bpp);
void unfilter_paeth(uint8_t *out, const uint8_t *filtered, const uint8_t *prev, size_t length, int bpp);

void filter_none(uint8_t *filtered, const uint8_t *scanline, size_t length);
void filter_sub(uint8_t *filtered, const uint8_t *scanline, size_t length, int bpp);
void filter_up(uint8_t *filtered, const uint8_t *scanline, const uint8_t *prev, size_t length);
void filter_avg(uint8_t *filtered, const uint8_t *scanline, const uint8_t *prev, size_t length, int bpp);
void filter_paeth(uint8_t *filtered, const uint8_t *scanline, const uint8_t *prev, size_t length, int bpp);

typedef enum 
{
	TF_R8,
	TF_RG8,
	TF_RGB233,
	TF_RGB332,
	TF_RGB8,
	TF_RGB565,
	TF_RGBA8,
	TF_RGBA4444,
	TF_RGBA5551,
	TF_RGBA1010102,
	TF_R16F,
	TF_RG16F,
	TF_RGB16F,
	TF_RGBA16F,
	TF_RGBA16,
	TF_BCN1,
	TF_BCN2,
	TF_BCN3,
	TF_BCN5,
	TF_BCN6N,
	TF_BCN7,
	TF_ETC1,
	TF_ETC2,
	TF_ETC4,
	TF_ASTC,	// pray
	TF_COMPRESS = 1 << 8
} tfss_formats;

typedef int (*mipmapper)(const uint8_t* in, uint8_t* out, uint32_t bpp, uint32_t wh[2]);

int mipmap_generic(const uint8_t* in, uint8_t* out, uint32_t bpp, uint32_t wh[2], uint8_t level);

int write_tfss(const char* name, char* data, uint32_t flags, uint32_t wh_dl[3], uint8_t format, uint8_t mip_count, uint8_t compression, uint8_t faces, mipmapper mipmap);
int read_tfss(const char* name, char* out, uint32_t* flags, uint32_t wh_dl[3], uint8_t* format, uint8_t* mip_count, uint8_t* faces);

#endif
