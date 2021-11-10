#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"

#include "AZ_device.h"




const char* AZ_desc = "{ name:\"AZ Instruments\", TAGS:[ { name:\"CO2\", unit:\"ppm\"},  { name:\"T\", unit:\"*C\"}, { name:\"RH\", unit:\"%\"}, { name:\"error\", unit:\"\"}]}";


int AZ_init()
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_enable_swap();

    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

    uart_enable_swap();

    return 0;
}


const char* AZ_get_desc()
{
  return AZ_desc;
}


size_t AZ_io( uint8_t* tx_buf, size_t tx_buf_len, uint8_t* rx_buf, size_t rx_buf_len, uint32_t timeout_ms)
{
    if( tx_buf == NULL
        || tx_buf_len == 0) {
        return -1;
    }

    uart_flush_input( UART_NUM_0);

    int res = uart_write_bytes( UART_NUM_0, (const char *)tx_buf, tx_buf_len);

    if( res != tx_buf_len) {
        return -1;
    }
    if( rx_buf == NULL
        || rx_buf_len == 0) {
        return 0;
    }

    // Read ACK from UART0.RX pin
    int ack_len = uart_read_bytes( UART_NUM_0, ( uint8_t*)rx_buf, rx_buf_len, ( TickType_t)( timeout_ms / portTICK_RATE_MS));
//if( ack_len > 0)
//printf( "RX: %.*s\n", (int)ack_len, rx_buf);
    return ack_len;
}


int AZ_get_SN( char* buf, size_t buf_len)
{
    if( buf == NULL
        || buf_len == 0) {
        return -1;
    }
/*
    request:  "I"
    response: "i 12345678 7798V3.4"
*/
    int ack_len = AZ_io( ( uint8_t*)"I", 1, ( uint8_t*)buf, ( buf_len-1), AZ_DEVICE_READ_TIMEOUT_MS);
    if( ack_len <= 0) {
        return -1;
    }
    buf[ ack_len] = 0;

    return ack_len;
}


int AZ_get_PV( AZ_PV* pv)
{
    char ack_data[ 32];

    if( pv == NULL) {
        return -1;
    }

/*
    request: ":"
    responds with : "T20.0C:C400ppm:H50.0%"
*/
    int ack_len = AZ_io( ( uint8_t*)":", 1, ( uint8_t*)ack_data, ( sizeof( ack_data)-1), AZ_DEVICE_READ_TIMEOUT_MS);
    if( ack_len <= 0) {
        return -1;
    }
    ack_data[ ack_len] = 0;
//printf( "ACK_LEN %d\n", ack_len);
//printf( "ACK %s\n", ack_data);

    char* next_tag = NULL;
    if( strncmp( ack_data, ": T", 3) != 0) {
        return -2;
    }
    pv->T = strtod( &ack_data[ 3], &next_tag);

    if( strncmp( next_tag, "C:C", 3) != 0) {
        return -3;
    }
    pv->CO2 = strtod( &next_tag[ 3], &next_tag);

    if( strncmp( next_tag, "ppm:H", 5) != 0) {
        return -4;
    }
    pv->RH = strtod( &next_tag[ 5], &next_tag);
        
    if( strncmp( next_tag, "%", 1) != 0) {
        return -5;
    }
//    pv->valid = true;
    return 0;
}


