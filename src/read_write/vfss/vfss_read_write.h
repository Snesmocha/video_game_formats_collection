#ifndef VFSS_FILE_CREATION_H
#define VFSS_FILE_CREATION_H

#include <stdint.h>

typedef enum
{
	VF_BYTE1 = 0,
	VF_BYTE2,
	VF_BYTE4,
	VF_BYTE8,
} vfss_byte_count;

typedef enum
{
	VF_UNIQ_NONE = 0,
	VF_UNIQ_10_2,
	VF_UNIQ_10_11,
} vfss_unique_format;

typedef struct
{
	char name[4];
	vfss_byte_count bytes;
	uint8_t is_float;
	uint8_t is_signed;
	vfss_unique_format is_unique;
	uint8_t offset;
	uint8_t component_count;
	uint8_t padding;
	uint8_t chain;
} vfss_vertex_attribute;

typedef struct
{
	char name[32];
	uint32_t vertex_count;
	uint32_t index_count;
	uint8_t* vertex;
	uint8_t* index;
} vfss_mesh;

int write_vfss(const char* name, uint8_t flags, const vfss_mesh* mesh, uint32_t mesh_count, const vfss_vertex_attribute* attributes, uint32_t attrib_count, vfss_byte_count index_attribute);
int read_vfss(const char* name, uint8_t* flags, vfss_mesh* mesh, uint32_t* mesh_count, vfss_vertex_attribute* attributes, uint32_t* attrib_count, vfss_byte_count* index_attribute);
#endif