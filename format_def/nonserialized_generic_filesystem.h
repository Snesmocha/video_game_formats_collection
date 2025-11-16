#ifndef NGSS
#define NGSS

#include <stdint.h>


#define DYNAMICS(N, T) \
		struct \
		{ \
			uint64_t size; \
			T* data; \
		} N##_arr
		

typedef enum
{
	T_U8,
	T_U16,
	T_U32,
	T_U64,
	
	T_S8,
	T_S16,
	T_S32,
	T_S64,
	
	T_F32,
	T_F64,
	
	
	T_U8_A,
	T_U16_A,
	T_U32_A,
	T_U64_A,
	
	T_S8_A,
	T_S16_A,
	T_S32_A,
	T_S64_A,
	
	T_F32_A,
	T_F64_A,
	T_STR, 
	T_STR_ARR, 
} ngfs_type;
typedef struct
{
	ngfs_type type;
	union
	{
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
		
		int8_t s8;
		int16_t s16;
		int32_t s32;
		int64_t s64;
		
		float f32;
		double f64;
		
		
		DYNAMICS(u8 , uint8_t );
		DYNAMICS(u16, uint16_t);
		DYNAMICS(u32, uint32_t);
		DYNAMICS(u64, uint64_t);
		
		DYNAMICS(s8 , int8_t );
		DYNAMICS(s16, int16_t);
		DYNAMICS(s32, int32_t);
		DYNAMICS(s64, int64_t);
		
		DYNAMICS(f32, float  );
		DYNAMICS(f64, double );
		
		struct
		{
			uint64_t size;
			char* data;
		} str;
		
		struct
		{
			uint64_t size;
			uint64_t* str_sizes;
			char** data;
		} str_array;
	} data;
} ngss_datatypes;


typedef struct
{
	ngfs_type type;
	char name[64]; // being honest you'll most likely NEVER see a name beyond 64 characters so whatever
	uint64_t size;
	uint64_t offset;
} data_map;


typedef struct
{
	char magic[4] = {'N', 'G', 'S', 'S'};
	
	uint64_t key_size;
	uint64_t compiled_data_size;
	data_map* map;
	
} ngss_compiled;


#endif