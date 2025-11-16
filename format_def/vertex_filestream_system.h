#ifndef VFSS_H
#define VFSS_H

#include <stdint.h>




typedef struct 
{
	char name[32];         // buffer name
	uint32_t vertex_size;  // Size of each vertex in vertex elements
	uint32_t index_size;   // Total size of index buffer
} VFSS_buffer_info;


//vertex filestream system
typedef struct 
{
	char magic[4];  				// "VFSS"
	uint8_t flags;  				// Flags for compression, endianess, etc.

	uint8_t vertex_attr_count;
	struct {
		char name[4];
		uint8_t format; // bits 1-2 : 0 for 1 byte, 1 for 2 bytes, 2 for 4 bytes, 3 for 8 bytes (avoid if possible)
						// third bit: 	   is floating point
						// forth bit:      is signed
						// bits 5-6 : 0 is none, 1 = 10*3_2, 2 = 10_11*2, 3 is undefined
		uint8_t offset;
		uint8_t component_count;
		uint8_t padding;
		uint8_t chain_flags; // so long as the byte format matches, 
							 // attributes can be chained but read as seperate like norm tan 
							 // or to attach an additional value to better pack values together such as xyz in f32 + u16 uv

	} attributes;

	uint8_t index_type; 			// VFSS_format must be unsigned

	char mesh_section[4];  			// "VFBS"
	uint32_t mesh_count;   			// Number of meshes
	VFSS_buffer_info* buffers;  	// Variable array of mesh buffers
} VFSS;
#endif
