#include "esp_log.h"
#include "tag.h"
#include "my_app.h"
#include "mqtt.h"
#include "AZ_device.h"



static const char *TAG = "LIGHT_APP";

static bool mqtt_is_connected = false;

esp_err_t httpd_register_user_uri_handlers( httpd_handle_t server)
{
	return ESP_OK;
}

esp_err_t mqtt_event_user_handler_cb( esp_mqtt_event_handle_t event)
{
//    esp_mqtt_client_handle_t client = event->client;
//    int msg_id;
//     your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
mqtt_is_connected = true;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
mqtt_is_connected = false;
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
//            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("MQTT_EVENT_DATA: %.*s = %.*s          \r", event->topic_len, event->topic, event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}



void tag_send( TAG_handle tag) {
    if( tag) {
        char text_buf[ 32];
        tag2str( tag, text_buf, sizeof( text_buf));
        mqtt_publish_data( tag->name, text_buf);
    }
}

enum{
    TAG_CO2_ID = 0,
    TAG_T_ID = 1,
    TAG_RH_ID = 2,
    TAG_ID_COUNT = 3
};
static void AZ_task() {
    AZ_PV pv;
    char SN_buf[ 40];
    char text_buf[ 64];
    AZ_init();
   
    TAG_handle tag_CO2 = tag_create();
    TAG_handle tag_T = tag_create();
    TAG_handle tag_RH = tag_create();
    
    tag_init( tag_CO2, 50, 10, "CO2", "ppm", "%5.0f");
    tag_init( tag_T, 0.5, 10, "T", "*C", "%4.1f");
    tag_init( tag_RH, 5, 10, "RH", "%", "%4.1f");

    TAG_handle tags[ 3];
    tags[ TAG_CO2_ID] = tag_CO2;
    tags[ TAG_T_ID] = tag_T;
    tags[ TAG_RH_ID] = tag_RH;
    for( int i = 0; i < TAG_ID_COUNT; i++) {
        if( tags[ i] == NULL) {
            ESP_LOGE( TAG, "Can't init tags[zero pointer]");
            vTaskDelete( NULL);
            return;
        }
    }

    while( 1) {
        printf( "MQTT wait connection ...\n");
        while( !mqtt_is_connected) {
            vTaskDelay( 1000 / portTICK_PERIOD_MS);
        }
        printf( "MQTT Connected\n");

        int err = AZ_get_SN( SN_buf, sizeof( SN_buf));
        if( err > 0) {
            printf( "AZ Read SN: %s\n", SN_buf);
        } else {
            sprintf( text_buf, "AZ_get_SN ERROR %d", err);
            printf( "%s\n", text_buf);
            mqtt_publish_error( text_buf);
        }
            
        mqtt_publish_data( "description", ( char*)AZ_get_desc());

        while( mqtt_is_connected) {
            int err = AZ_get_PV( &pv);
            if( err == 0) {
                printf( "AZ Readed: CO2=%d, T=%5.1f, RH=%5.1f\n", ( int)pv.CO2, pv.T, pv.RH);

                tag_update( tag_CO2, pv.CO2);
                tag_update( tag_T, pv.T);
                tag_update( tag_RH, pv.RH);

                // Find *_motivation >= 1: send data and reset else break
                for( int i = 0; i < TAG_ID_COUNT; i++) {
                    if( ( tags[ i]->time_motivation >= 1) || ( tags[ i]->value_motivation >= 1)) {
                        tag_send( tags[ i]);
                        tag_reducer_reset( tags[ i]);

                        // add if( ( time_motivation + value_motivation) > 1) : send data and reset 
                        for( int j = 0; j < TAG_ID_COUNT; j++) {
                            if( ( tags[ j]->time_motivation + tags[ j]->value_motivation) >= 1) {
                                tag_send( tags[ j]);
                                tag_reducer_reset( tags[ j]);
                            }
                        }
                        break;
                    }
                }
            } else {
                sprintf( text_buf, "AZ_get_PV ERROR %d", err);
                printf( "%s\n", text_buf);
                mqtt_publish_error( text_buf);
            }

            vTaskDelay( 4000 / portTICK_PERIOD_MS);
        }
        printf( "MQTT Disconnected\n");
    }
    vTaskDelete(NULL);
}


esp_err_t user_app_main()
{
//    TickType_t xTaskGetTickCount();
    xTaskCreate( AZ_task, "AZ_task", 2048, NULL, 10, NULL);
	return ESP_OK;
}


