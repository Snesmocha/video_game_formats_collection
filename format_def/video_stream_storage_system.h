#ifndef VSSS
#define VSSS

#include <stdint.h>


typedef struct
{
  uint32_t timecode_scale;
  uint32_t duration;
} segment;

typedef struct
{
  uint32_t width;
  uint32_t height;
  float framerate;
} video_info;


typedef struct
{
  float sampling_frequency;
  uint8_t channels;
} audio_info;

typedef struct
{
  uint32_t track_index;
  uint8_t type;
  uint8_t codec;
  union
  {
    audio_info audio;
    video_info video;
  };
  
} track_entry;


typedef struct 
{
  uint32_t track_count;
  track_entry* entries;
} tracks;

typedef struct 
{
  char magic[4]; // VSSS
  
  
} VSSS;

#endif
