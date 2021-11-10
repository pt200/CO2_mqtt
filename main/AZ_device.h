/*
    AZ device IO
        Port settings: 9600 8N1
        The 3-way, 2.5mm jack has tip=RxD, middle=TxD and base=0V
        The TxD output swing is 3V3.

        get serial number / ID
            request:  "I"
            response: "i 12345678 7798V3.4"

        get PV
            request: ":"
            response: "T20.0C:C400ppm:H50.0%"
*/
/*
*   Send Request ":" to UART0.TX pin
*   Read ACK from UART0.RX pin
*   UART1.TX using for logging
*/

#ifndef __AZ_DEVICE_H__
#define __AZ_DEVICE_H__

#include <stdlib.h>


#ifdef __cplusplus
extern "C"
{
#endif
 

#define BUF_SIZE (256)
#define AZ_DEVICE_READ_TIMEOUT_MS    (1000)


typedef struct{
    float CO2;
    float T;
    float RH;
}AZ_PV;


/**
 * @brief Init driver
 *
 * @return
 *     - 0
 */
int AZ_init();


/**
 * @brief Request data from device
 *
 * @param tx_buf     xxxxxx
 * @param tx_buf_len  xxxxxx
 *
 * @return
 *     - -1     ERROR
 *     - >=0    Readed bytes in rx_buf
 */
size_t AZ_io( uint8_t* tx_buf, size_t tx_buf_len, uint8_t* rx_buf, size_t rx_buf_len, uint32_t timeout_ms);


/**
 * @brief 
 *
 * @return
 */
const char* AZ_get_desc();


/**
 * @brief Request info from device
 *
 * @param buf       Buffer for ack data( returned zero terminated string)
 * @param buf_len   Length of buffer
 *
 * @return
 *     - -1     ERROR
 *     - >=0    Readed bytes in buf
 */


int AZ_get_SN( char* buf, size_t buf_len);


/**
 * @brief Request process variables from device
 *
 * @param pv       pointer to AZ_PV struct for returning data
 *
 * @return
 *     - 0      OK
 *     - <0     ERROR
 */
int AZ_get_PV( AZ_PV* pv);


#ifdef __cplusplus
}
#endif

#endif
