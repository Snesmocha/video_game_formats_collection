#ifndef VFSS_H
#define VFSS_H

#include <stdint.h>



typedef enum 
{
    // Vertex attribute formats (e.g., position, normal, etc.)
	VF_U8,
	VF_U16,
	VF_U32,
	VF_S8,
	VF_S16,
	VF_S32,
} VFSS_format;

typedef struct 
{
	char name[32];         // Optional buffer name
	uint32_t vertex_size;  // Size of each vertex in bytes
	uint32_t index_size;   // Total size of index buffer
} VFSS_buffer_info;


//vertex filestream system
typedef struct 
{
	char magic[4];  				// "VFSS"
	uint8_t flags;  				// Flags for compression, endianess, etc.

	uint8_t vertex_attr_count; 		// Number of attributes (max 64)
	struct {
		uint8_t flags; 
		uint8_t format;      		// Float3, Byte4, etc.
		uint8_t offset;      		// Offset in each vertex
		uint8_t padding;     		// Alignment padding
	} attributes[64];

	uint8_t index_type; 			// VFSS_format must be unsigned

	char mesh_section[4];  			// "VFBS"
	uint32_t mesh_count;   			// Number of meshes
	VFSS_buffer_info* buffers;  	// Variable array of mesh buffers
} VFSS;
#endif
