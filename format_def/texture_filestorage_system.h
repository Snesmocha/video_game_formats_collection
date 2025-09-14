#ifndef TFSS_H
#define TFSS_H

#include <stdint.h>

/*

00000000 00000000 00000000 00000000  

first two bits are used to determine graphics api for format
whatever is placed into format is determined by the graphics api / end user
*/

typedef enum
{
	TF_CUBEMAP = 1 << 0,
	TF_CUBEMAP_ARRAY = 1 << 1,
	TF_3D_OR_ARRAY = 1 << 2,
	
} texture_flags;

typedef enum 
{
	TF_R8,
	TF_RG8,
	TF_RGB233,
	TF_RGB332,
	TF_RGB8,
	TF_RGB565,
	TF_RGBA8,
	TF_RGBA4444.
	TF_RGBA5551.
	TF_RGBA1010102.
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
} texture_formats;

typedef struct
{
	uint32_t length; // size of initial image
	uint32_t offset; // reletive
} additional_metadata;

// textufe file storage system
typedef struct
{
	char magic[4]; 	// TFSS

	uint32_t flags;
    uint32_t width;            // Texture width
    uint32_t height;           // Texture height
	uint32_t depth;			  // Texture depth or array
	
	uint8_t format;	
	uint8_t compression;
	uint8_t mip_count;
	uint8_t face_count;
	
	// size is calculated by wh_dl[2] * mip_count
	// note zero mip map means no mip map and 1 is one mipmap so on and so on, but zero or 1 depth or layer is only a single image
	// additional_metadata* image_offset
	
	// allignment is calculated the same way as ktx for the most part
	// note if compressed, this must be decrypted before the this can be handled
	// mip maps are stored side by side to the texture data
	// if the texture size is uneven in any way, it will attempt to allign 
	/*
		allign( lcm(format_size, 4) ) mip pad
		for array index in max(1, array)
		{
			for face in face count
			{
				for mip in mipcount
					for d / mip for h / mip for w / mip
						bytes data[format_size];
			}
		}
	*/
} TFSS;

#endif
