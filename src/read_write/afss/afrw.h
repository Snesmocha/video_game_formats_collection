#ifndef AFSS_RW
#define AFSS_RW

#include <audio_format_system_individual_storage_container.h>
#include <wavpack/wavpack.h>

#include <stdio.h>

typedef struct
{
  int size;
  file_atlas* atlas;
} atlas;


typedef enum
{
  AFSS_WRITE_HEADER,
  AFSS_WRITE_ATLAS,
  AFSS_WRITE_AUDIO,
  AFSS_WRITE_FINISHED,
  AFSS_ERROR
} afss_state;

int afss_write_start(FILE* fh, afas* header, const char* filename, int compression_level);
int afss_write_atlas_index(FILE* fh, afas* header, char name[32], char catagory[32]);
int afss_afsi_write(FILE* fh, file_atlas location, afsi* header,loop_data* loop, marker_data* marker, const char* filename, const char* audio_filename);
int afss_write_end(FILE*);


// writes an individual afsi file
int afsi_write_file(afsi* header, loop_data* loop, marker_data* marker, const char* filename, const char* audio_filename);


atlas afss_get_atlas(const char* filename);
afsi afsi_from_afss_atlas(const char* filename, atlas* lookup, const char name[32]);
afsi afsi_from_afss_atlas_c(const char* filename, atlas* lookup, const char name[32]);

afsi afsi_get_header(const char* filename);




#endif
