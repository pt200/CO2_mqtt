#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

typedef struct{
//    Mutex lock;
    float value;
    float published_value;
    int tick_cnt;

    float time_motivation;
    float value_motivation;

    float value_treshold;
    int max_tick;

    char* name;
    char* format_string; // for printf
    char* eunt;
}TAG_t;
typedef TAG_t* TAG_handle;


TAG_handle tag_create();
void tag_destroy( TAG_handle tag);
void tag_init( TAG_handle tag, float value_treshold, int max_tick,
               const char* name, const char* eunt, const char* format_string);
void tag_deinit( TAG_handle tag);
void tag_reducer_reset( TAG_handle tag);
void tag_update( TAG_handle tag, float value);
void tag2str( TAG_handle tag, char* buf, size_t len);

#ifdef __cplusplus
}
#endif

