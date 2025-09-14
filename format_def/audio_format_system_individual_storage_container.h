#ifndef AFSS
#define AFSS

//audio file storage system


#include <stdint.h>

#define MAX_FILESIZE 4.295e9

// all other data is defined in wavpack, along with features and such
typedef struct 
{
  char ckID [4]; // "wvpk"
  uint32_t ckSize; // size of entire block (minus 8)
  uint16_t version; // 0x402 to 0x410 are valid for decode
  char block_index_u8; // upper 8 bits of 40-bit block_index
  char total_samples_u8; // upper 8 bits of 40-bit total_samples
  uint32_t total_samples; // lower 32 bits of total samples for
  
  // entire file, but this is only valid
  // if block_index == 0 and a value of -1
  // indicates an unknown length
  uint32_t block_index; // lower 32 bit index of the first sample
  // in the block relative to file start,
  // normally this is zero in first block
  uint32_t block_samples; // number of samples in this block, 0 =
  // non-audio block
  uint32_t flags; // various flags for id and decoding
  uint32_t crc; // crc for actual decoded data
} wavpack_header;


typedef enum
{
    LOOP_NONE,
    LOOP_FORWARD,      // Standard looping
    LOOP_PINGPONG      // Alternates direction at each loop
} loop_mode;


// 8 bytes
typedef struct 
{
    float time;
    uint32_t id; // Or a string hash
} marker_data;


// 32 bytes
typedef struct
{
  uint32_t loop_index;    
  
  float loop_start, loop_end;
  loop_mode mode;
  uint32_t loop_count;  // -1 if infinite

  float fadein_dur;
  float fadeout_dur;

  float crossfade_dur;
}loop_data;


// audio file stream 32 bytes 
typedef struct
{
  char magic[4];  // AFSI

  uint32_t flags;

  float gain_db;
  float pitch_shift;
  float pan;
  uint32_t priority;
  
  uint32_t loop_points_size; // if = max value of 4,294,967,295, then there are no loop points
  // loop data will go here
  uint32_t markers;          // if = max value of 4,294,967,295, then there are no markers points
  
  wavpack_header audiofile_header;
} afsi;

typedef struct
{
  char name[32];
  char catagory[24];
  uint64_t offset;
} file_atlas;


// supports zstd compression, 13 bytes
typedef struct 
{
  char magic[4]; // AFSS
  uint8_t compression_level; // if 0 then no commpression, 
  uint64_t audiofile_count;  // imediately after this file is the name, catagory, and file ofofset of every file in this pack
} afas;


#endif
