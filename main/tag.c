#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "esp_log.h"

#include "tag.h"


static const char *TAG = "TAG_COMPONENT";
#define TAG_LOGI( format, ... )  ESP_LOGI( TAG, format, ##__VA_ARGS__)
#define TAG_LOGE( format, ... )  ESP_LOGE( TAG, format, ##__VA_ARGS__)



TAG_handle tag_create() {
    TAG_handle tag;
    tag = malloc( sizeof( TAG_t));
TAG_LOGI( "tag_create => %d", ( int)tag);
    return tag;
}

void tag_destroy( TAG_handle tag) {
TAG_LOGI( "tag_destroy( %d)", ( int)tag);
    tag_deinit( tag);
    if( tag) {
        free( tag);
        tag = NULL;
    }
}

void tag_deinit( TAG_handle tag) {
TAG_LOGI( "tag_deinit( %d)", ( int)tag);
    if( tag) {
        free( tag->name);
        tag->name = NULL;
        free( tag->format_string);
        tag->format_string = NULL;
        free( tag->eunt);
        tag->eunt = NULL;
    }
}

void tag_init( TAG_handle tag, float value_treshold, int max_tick,
               const char* name, const char* eunt, const char* format_string) {
    if( tag) {
        tag->value = 0;
        tag->published_value = 0;
        tag->tick_cnt = 0;
        tag->time_motivation = 0;
        tag->value_motivation = 0;
        tag->value_treshold = value_treshold;
        tag->max_tick = max_tick;
        tag->name = strdup( name);
        tag->format_string = strdup( format_string);
        tag->eunt = strdup( eunt);
    }
}

void tag_reducer_reset( TAG_handle tag) {
    if( tag) {
        tag->published_value = tag->value;
        tag->tick_cnt = 0;
        tag->time_motivation = 0;
        tag->value_motivation = 0;
    }
}

void tag_update( TAG_handle tag, float value) {
TAG_LOGI( "tag_update( %d, %f)", ( int)tag, value);
    if( tag) {
        tag->value = value;
        tag->tick_cnt++;
        if( tag->value_treshold > 0) {
            tag->value_motivation = fabs( tag->value - tag->published_value) / tag->value_treshold;
        }else{
            tag->value_motivation = 1;
        }
        if( tag->max_tick > 0) {
            tag->time_motivation = ( float)tag->tick_cnt / tag->max_tick;
        }else{
            tag->time_motivation = tag->tick_cnt;//1;
        }
TAG_LOGI( "    value_motivation = %f,  time_motivation = %f", tag->value_motivation, tag->time_motivation);
    }
}

void tag2str( TAG_handle tag, char* buf, size_t len) {
TAG_LOGI( "tag2str( %d)", ( int)tag);
    if( tag) {
        if( tag->format_string) {
            snprintf( buf, len, tag->format_string, tag->value);
        } else {
            snprintf( buf, len, "%f", tag->value);
        }
TAG_LOGI( "    value = %s", buf);
    }
}

