
#ifndef BLE_WECHAT_SERVICE
#define BLE_WECHAT_SERVICE

#include <string.h>
#include <stdint.h>
#include "ble.h"
//#include "nrf_gpio.h"
#include "ble_conn_params.h"
#include "ble_stack_handler.h"
#include "ble_types.h"
#include "ble_wechat_util.h"

#define BLE_UUID_WECHAT_SERVICE							 			0xFEE7
#define BLE_UUID_WECHAT_WRITE_CHARACTERISTICS 		0xFEC7
#define BLE_UUID_WECHAT_INDICATE_CHARACTERISTICS 	0xFEC8
#define BLE_UUID_WECHAT_READ_CHARACTERISTICS 			0xFEC9
//#define APP_ADV_INTERVAL                     40                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
//#define APP_ADV_TIMEOUT_IN_SECONDS           0                                       /**< The advertising timeout in units of seconds. */
#define BLE_WECHAT_MAX_DATA_LEN            (GATT_MTU_SIZE_DEFAULT - 3) 							/**<Set the Maximum Data length og WECHAT to the (GATT Maximum Transmission Unit -3) */


typedef struct
{
		uint16_t                   service_handle;
		ble_gatts_char_handles_t   indicate_handles;
		ble_gatts_char_handles_t   write_handles;
		ble_gatts_char_handles_t   read_handles;
		uint16_t                   conn_handle; 
} ble_wechat_t;
/**服务结构体**/

typedef struct
{
		uint8_t *data;
		uint16_t len;
		uint16_t offset;
} data_info;
/**数据结构体**/


void ble_wechat_on_ble_evt(ble_wechat_t *p_wcs,ble_evt_t * p_ble_evt);
uint32_t ble_wechat_add_service(ble_wechat_t *p_wechat);
uint32_t ble_wechat_add_characteristics(ble_wechat_t *p_wechat);
int ble_wechat_indicate_data(ble_wechat_t *p_wcs, uint8_t *data, int len);
void get_mac_addr(uint8_t *p_mac_addr);

#endif
