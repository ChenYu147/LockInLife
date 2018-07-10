/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf51_bitfields.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "app_trace.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "ble_wechat_service.h"
#include "aes.h"
#include "base64.h"
#include "main.h"
#include "data_processing.h"
#include "key.h"
#include "motor.h"
#include "app_uart.h"
#include "flash.h"
#include "ad.h"
#include "wdt.h"
#include "ble_wechat_service.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT  1                                          /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define DEVICE_NAME                      "WeChat"                               /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                "NordicSemiconductor"                      /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                 1600                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 1 s). */
#define APP_ADV_TIMEOUT_IN_SECONDS       0                                        /**< The advertising timeout in units of seconds. */

#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_MAX_TIMERS             (6+BSP_APP_TIMERS_NUMBER)                  /**< Maximum number of simultaneously created timers. */
#define APP_TIMER_OP_QUEUE_SIZE          4                                          /**< Size of timer operation queues. */

#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(200, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(250, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                    0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                         /**< Maximum encryption key size. */

#define DEAD_BEEF                        0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)    /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define TIMER_INTERVAL									 APP_TIMER_TICKS(100,APP_TIMER_PRESCALER)		/**  (MS,) **/

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */


static dm_application_instance_t        m_app_handle;                               /**< Application identifier allocated by device manager */

static uint32_t                          m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
static  ble_wechat_t 													m_ble_wechat;

//static uint8_t	m_addl_adv_manuf_data[BLE_GAP_ADDR_LEN];
uint32_t time_num=0;
uint8_t session_key[16] = {0};
extern uint8_t read_time_flag;
extern uint8_t	m_addl_adv_manuf_data[BLE_GAP_ADDR_LEN];
extern receive_data_info receive_data;	
unsigned char receive_Plaindata[50]={0};		//接收到解密后的数据
extern uint8_t motor_state_key_flag;
unsigned char receive_phone_num[11]={0};		//接收到手机号码

unsigned char receive_time_num[13]={0};			//接收到时间戳
unsigned char receive_super_time_num[13]={0};			
uint8_t password_num[8]={0x01,0x02,0x03,0x01,0x02,0x03};
uint8_t super_password_num[8]={0x01,0x02,0x03,0x01,0x02,0x03};
uint8_t flash_phone_num[12]={0x01,0x02,0x03,0x01,0x02,0x03,0x01,0x02,0x03,0x01,0x02,0x03};			//flash存的手机号码
uint8_t aaas[100]={0};
extern uint8_t send_end_flag;
uint8_t receive_data_12_flag=0,receive_data_13_flag=0;
char send_Plaindata11[50]="0x11133001188688320450000186400";		//发送加密后的数据
char send_Plaindata12[50]="0x121";		//发送加密后的数据
char send_Plaindata13[50]="0x131";
char send_Plaindata14[50]="0x141";
uint16_t ad_num=0;
uint8_t start_string[20]="Start...\n";
extern uint8_t g_rcv_data_data[120]; 


/* YOUR_JOB: Declare all services structure your application is using
static ble_xx_service_t                     m_xxs;
static ble_yy_service_t                     m_yys;
*/

// YOUR_JOB: Use UUIDs for service(s) used in your application.
//static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */
static ble_uuid_t m_adv_uuids[] = {{0xFEE7, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */




                                   
/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

char	key_time_flag=0,motor_time_flag=0,disconnect_delay_flag=0,motor_delay_flag=0,KEY_OPEN_dleay_flag=0,KEY_OPEN_flag=0,DEEP_OPEN_dleay_flag=0,LOCK_status_flag=0,KEY_OPEN_dleay_one_flag=0,LOCK_status_use=0;
int	receive_delay_flag=0,receive_delay_num=0,disconnect_delay_num=0,motor_delay_num=0,KEY_OPEN_dleay_num=0,DEEP_OPEN_dleay_num=0;
long long use_time_num=0;

//100 MS进一次
static void timer_timeout_handler(void * p_context)
{
	uint32_t err_code;
	UNUSED_PARAMETER(p_context);
	time_num++;
	if(time_num % 1 == 0)
	{
		key_time_flag=1;
	}
	if(time_num % 2 == 0)
	{
		motor_time_flag=1;
	}
	if(receive_delay_flag==1)
	{
		receive_delay_num++;
		if(receive_delay_num>2)
		{
			receive_delay_flag=0;
			receive_delay_num=0;
		}
	}
	if(	time_num % 10 == 0)
	{	
		if( read_time_flag == 1 )use_time_num++;
		time_num = 0;
	}
		
	if(disconnect_delay_flag==1)
	{
		disconnect_delay_num++;
		if(disconnect_delay_num>10)
		{
			disconnect_delay_flag=0;
			disconnect_delay_num=0;
			err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
			APP_ERROR_CHECK(err_code);
			printf("\n BLE connected \n");
		}
	}
	if(motor_delay_flag==1)
	{
		motor_delay_num++;
		if(motor_delay_num>12)
		{
			motor_delay_flag=0;
			motor_delay_num=0;
			motor_state_key_flag = OFF;		//上锁
			receive_data_12_flag=0;
		}
	}
	if(KEY_OPEN_dleay_flag==1 && KEY_OPEN_dleay_one_flag==0)		
	{
		KEY_OPEN_dleay_num++;
		if(KEY_OPEN_dleay_num>50 )
		{
			KEY_OPEN_dleay_num=0;
			KEY_OPEN_dleay_one_flag=1;
			KEY_OPEN_flag=1;
		}
	}
	else KEY_OPEN_dleay_num=0;
	
	if(DEEP_OPEN_dleay_flag==1)
	{
		DEEP_OPEN_dleay_num++;
		nrf_gpio_pin_set(BEEP);											//蜂鸣器响
		if(DEEP_OPEN_dleay_num>=1800)
		{
			DEEP_OPEN_dleay_num=0;
			DEEP_OPEN_dleay_flag=0;
			nrf_gpio_pin_clear(BEEP);											//蜂鸣器不响
		}

	}
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static app_timer_id_t m_app_timer_id;
static void timers_init(void)
{

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
    uint32_t err_code;
    err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
    APP_ERROR_CHECK(err_code); */
	
	  uint32_t err_code;
    err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
    APP_ERROR_CHECK(err_code);
	
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);			//安全模式设置

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));				//设置设备名称
		
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
    APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the YYY Service events. 
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
static void on_yys_evt(ble_yy_service_t     * p_yy_service, 
                       ble_yy_service_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. \r\n", p_evt->params.char_xx.value.p_str);
            break;
        
        default:
            // No implementation needed.
            break;
    }
}*/

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    /* YOUR_JOB: Add code to initialize the services used by the application.
    uint32_t                           err_code;
    ble_xxs_init_t                     xxs_init;
    ble_yys_init_t                     yys_init;

    // Initialize XXX Service.
    memset(&xxs_init, 0, sizeof(xxs_init));

    xxs_init.evt_handler                = NULL;
    xxs_init.is_xxx_notify_supported    = true;
    xxs_init.ble_xx_initial_value.level = 100; 
    
    err_code = ble_bas_init(&m_xxs, &xxs_init);
    APP_ERROR_CHECK(err_code);

    // Initialize YYY Service.
    memset(&yys_init, 0, sizeof(yys_init));
    yys_init.evt_handler                  = on_yys_evt;
    yys_init.ble_yy_initial_value.counter = 0;

    err_code = ble_yy_service_init(&yys_init, &yy_init);
    APP_ERROR_CHECK(err_code);
    */
		
		uint32_t err_code;
		err_code = ble_wechat_add_service(&m_ble_wechat);
		APP_ERROR_CHECK(err_code);
		err_code = ble_wechat_add_characteristics(&m_ble_wechat);
		APP_ERROR_CHECK(err_code);
			
		
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
*/
static void application_timers_start(void)
{
    /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
    uint32_t err_code;
    err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code); */
	
 	  uint32_t err_code;
    err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);  
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

//    // Prepare wakeup buttons.
//    err_code = bsp_btn_ble_sleep_mode_prepare();
//    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
            {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
						printf("\n APP connected \n");
						nrf_gpio_pin_clear(LED_BLUE);				//蓝灯亮
//						nrf_gpio_pin_clear(LED_BLUE);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
						printf("\n APP disconnected \n");
						nrf_gpio_pin_set(LED_BLUE);				//蓝灯亮
//						nrf_gpio_pin_set(LED_BLUE);
            break;
								
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
		ble_wechat_on_ble_evt(&m_ble_wechat, p_ble_evt);
		dm_ble_evt_handler(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    /*YOUR_JOB add calls to _on_ble_evt functions from each service your application is using
    ble_xxs_on_ble_evt(&m_xxs, p_ble_evt);
    ble_yys_on_ble_evt(&m_yys, p_ble_evt);
    */

}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
//    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);
		SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_250MS_CALIBRATION,  NULL);	//不使用外部32.768K晶振  PPM 误差 百万分之一

#if defined(S110) || defined(S130)
    // Enable BLE stack.
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
#ifdef S130
    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_DEFAULT;
#endif
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
#endif

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
    APP_ERROR_CHECK(event_result);

#ifdef BLE_DFU_APP_SUPPORT
    if (p_event->event_id == DM_EVT_LINK_SECURED)
    {
        app_context_load(p_handle);
    }
#endif // BLE_DFU_APP_SUPPORT

    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;

		ble_advdata_manuf_data_t manuf_specific_data;
	
	  manuf_specific_data.data.p_data = m_addl_adv_manuf_data;
    manuf_specific_data.data.size   = 6;
		manuf_specific_data.company_identifier =0x534a;
	
    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_SLOW_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;			//设为0 无广播超时

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

void get_AES_KEY(unsigned char *pKey)					//得到AES的密钥 0000+MAC地址
{
	char temp[2];		
	for(int i=0; i<4; i++)
	{
		pKey[i]='0';		
	}
	for(int i=0; i<6; i++)
	{    
		sprintf(temp, "%x", m_addl_adv_manuf_data[i]);
		if(temp[1]==0x00){
			pKey[4+2*i]=0x30;
			pKey[4+2*i+1]=temp[0];
		}
		else
		{			
				pKey[4+2*i]=temp[0];
				pKey[4+2*i+1]=temp[1];
		}
	}
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    nrf_gpio_cfg_output(LED_RED);
    nrf_gpio_cfg_output(LED_BLUE);
	  nrf_gpio_cfg_output(BEEP);
	  nrf_gpio_pin_set(LED_RED);
		nrf_gpio_pin_set(LED_BLUE);
		nrf_gpio_pin_clear(BEEP);
}

void Holzer_init(void)
{
	nrf_gpio_cfg_sense_input(Holzer1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);					//霍尔1 设置唤醒
	nrf_gpio_cfg_sense_input(Holzer2, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);					//霍尔2 设置唤醒
}

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to 
 *          a string. The string will be be sent over BLE when the last character received was a 
 *          'new line' i.e '\n' (hex 0x0D) or if the string has reached a length of 
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
//    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
//    static uint8_t index = 0;
//    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
//            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
//            index++;

//            if ((data_array[index - 1] == '\n') || (index >= (BLE_NUS_MAX_DATA_LEN)))
//            {
//                err_code = ble_nus_string_send(&m_nus, data_array, index);
//                if (err_code != NRF_ERROR_INVALID_STATE)
//                {
//                    APP_ERROR_CHECK(err_code);
//                }
//                
//                index = 0;
//            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
/**@snippet [Handling the data received over UART] */


/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud38400
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    APP_ERROR_CHECK(err_code);
}
/**@snippet [UART Initialization] */


/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
    bool erase_bonds;
	
//		/** 设置32M外部晶振 **/
//		NRF_CLOCK->XTALFREQ=0x00;
//		NRF_UICR->XTALFREQ=0x00;
//	
    // Initialize.
	  leds_init();
		Holzer_init();
//	  uart_init();   降低功耗
	  printf("\n %s \n",start_string);	
	  key_init();	
		motor_init();
    timers_init();
    ble_stack_init();
    device_manager_init(erase_bonds);
    gap_params_init();
		get_mac_addr(m_addl_adv_manuf_data);		//获取MAC地址
    advertising_init();
    services_init();
    conn_params_init();
	
		flash_init();				//flash 初始化
		
//		adc_init();					//ADC初始化   降低功耗
		/******** 设置看门狗 ********/
		wdt_init();					//看门狗初始化
		wdt_start();				
		
    // Start execution.
    application_timers_start();
    err_code = ble_advertising_start(BLE_ADV_MODE_SLOW);
    APP_ERROR_CHECK(err_code);
	
		get_AES_KEY(session_key);		
		printf("\n session_key:%s \n",session_key);

		/*************		 	FLASH操作						**************/					 
		receive_phone_num_read(flash_phone_num);
		if(flash_phone_num[0]==0xff)
		{
			for(int x=0;x<12;x++)flash_phone_num[x]=0x30;
		}	
		printf("\n flash read phone data:%s \n",flash_phone_num);
		
		receive_password_num_read(password_num);
		if(password_num[0]==0xff)
		{
			for(int x=0;x<8;x++)password_num[x]=0x30;
		}
		printf("\n flash read password data:%s \n",password_num);		

    // Enter main loop.
    for (;;)
    {	
			wdt_feed();
			if(receive_data.flag==1)
			{	
				printf("\n receive data:%s \n",g_rcv_data_data);
				receive_data_Decrypt(receive_Plaindata, receive_data);
				printf("\n receive Plaindata:%s \n",receive_Plaindata);
				
				if(receive_Plaindata[0] == '0' && receive_Plaindata[1] == 'x' && receive_Plaindata[2] == '1' && receive_Plaindata[3] == '1' && send_end_flag==1)
				{
					send_end_flag=0;	
					adc_enable();		//初始化ADC				
					if(LOCK_status_flag==0)ad_num=nrf_adc_read()*5;//3.3V---2560
					else ad_num=800;
//					adc_disable();	//失能ADC
					send_Plaindata11[5]=ad_num/1000+0x30;
					send_Plaindata11[6]=ad_num%1000/100+0x30;
					send_Plaindata11[7]=ad_num%100/10+0x30;
					send_Plaindata11[8]=ad_num%10+0x30;		
												
					if(nrf_gpio_pin_read(Holzer1) == 0 && nrf_gpio_pin_read(Holzer2) == 0)	//上锁 手机号码和时长为0
					{
						send_Plaindata11[9]=0x32;					//上锁状态
						for(int x=0;x<21;x++)							//上锁状态全部置0
						{
							send_Plaindata11[x+10]=0x30;
						}
					}
					else																//未锁
					{
						send_Plaindata11[9]=0x31;					//未锁状态
						if(LOCK_status_use==1)
						{
							for(int x=0;x<11;x++)
							{
								send_Plaindata11[x+10]=flash_phone_num[x];		//手机号码
							}					
							send_Plaindata11[30]=use_time_num%10+0x30;							//时间值
							send_Plaindata11[29]=use_time_num%100/10+0x30;					
							send_Plaindata11[28]=use_time_num%1000/100+0x30;
							send_Plaindata11[27]=use_time_num%10000/1000+0x30;
							send_Plaindata11[26]=use_time_num%100000/10000+0x30;
							send_Plaindata11[25]=use_time_num%1000000/100000+0x30;
							send_Plaindata11[24]=use_time_num%10000000/1000000+0x30;
							send_Plaindata11[23]=use_time_num%100000000/10000000+0x30;
							send_Plaindata11[22]=use_time_num%1000000000/100000000+0x30;
							send_Plaindata11[21]=use_time_num%10000000000/1000000000+0x30;								
						}
						else 						
						{
							for(int x=0;x<21;x++)							//锁还在使用中
							{
								send_Plaindata11[x+10]=0x30;
							}
						}
							
					}	
					
					char send_11_data[50];						
					send_data_Encrypt(send_Plaindata11,send_11_data,31);		
					ble_wechat_indicate_data(&m_ble_wechat,(uint8_t *)send_11_data,send_11_data[1]);	
					printf("\n send data:%s \n",send_11_data);	
					
				}
				if(receive_Plaindata[0] == '0' && receive_Plaindata[1] == 'x' && receive_Plaindata[2] == '1' && receive_Plaindata[3] ==  '2' && send_end_flag==1)
				{	
					send_end_flag=0;
					for(int x=0;x<11;x++)
					{
						flash_phone_num[x]=receive_Plaindata[x+4];
					}
					for(int y=0;y<13;y++)
					{
						receive_time_num[y]=receive_Plaindata[y+15];
					}	
					
					get_password(flash_phone_num,receive_time_num,m_addl_adv_manuf_data,password_num);
					printf("\n password data:%s \n",password_num);			//生成密码

					/*************		 	FLASH操作						**************/		
					receive_password_num_update(password_num);
					receive_phone_num_update(flash_phone_num);	
					
					char send_12_data[50];
					send_data_Encrypt(send_Plaindata12,send_12_data,5);		
					ble_wechat_indicate_data(&m_ble_wechat,(uint8_t *)send_12_data,send_12_data[1]);		
					printf("\n send data:%s \n",send_12_data);
					
					use_time_num=0;													//计时清0
					read_time_flag = 1;							//开始计时
					LOCK_status_use= 0;							
					motor_state_key_flag = ON;			//开锁
					receive_data_12_flag=1;					//开锁后，判断是否要上锁				
				}	
				if(receive_Plaindata[0] == '0' && receive_Plaindata[1] == 'x' && receive_Plaindata[2] == '1' && receive_Plaindata[3] == '3' && send_end_flag==1)//&& receive_delay_flag==0
				{	
					send_end_flag=0;
					char send_13_data[50];
					
					for(int y=0;y<13;y++)
					{
						receive_super_time_num[y]=receive_Plaindata[y+4];
					}	
					printf("\n receive super time num:%s \n",receive_super_time_num);
					
					get_super_password(receive_super_time_num,m_addl_adv_manuf_data,super_password_num);
					printf("\n super password data:%s \n",super_password_num);	
					
					send_data_Encrypt(send_Plaindata13,send_13_data,5);		
					ble_wechat_indicate_data(&m_ble_wechat,(uint8_t *)send_13_data,send_13_data[1]);
					printf("\n send data:%s \n",send_13_data);
					
					receive_data_13_flag=1;					//允许读取按键 超级密码
				}
				if(receive_Plaindata[0] == '0' && receive_Plaindata[1] == 'x' && receive_Plaindata[2] == '1' && receive_Plaindata[3] == '4' && send_end_flag==1)//&& receive_delay_flag==0
				{	
					send_end_flag=0;
					char send_14_data[50];
					send_data_Encrypt(send_Plaindata14,send_14_data,5);		
					ble_wechat_indicate_data(&m_ble_wechat,(uint8_t *)send_14_data,send_14_data[1]);
					printf("\n send data:%s \n",send_14_data);		
					disconnect_delay_flag=1;				//等待1S断开连接
				}
				receive_data.flag=0;			
				memset(receive_data.data,0,120);	//清0
				memset(receive_Plaindata,0,50);		//清0
				memset(g_rcv_data_data,0,120);		//清0			
			}
			
			if(LOCK_status_flag==0)							//锁可用
			{
				if(key_time_flag == 1 && nrf_gpio_pin_read(Holzer1) == 0 && nrf_gpio_pin_read(Holzer2) == 0)	//上锁状态就判断按键
				{
					if(receive_data_13_flag == 1)key_super_run();						//判断超级密码
					else key_run();												//判断密码
					key_time_flag =0;
				}
				
				if(motor_time_flag)																											//40MS进一次
				{
					if(motor_state_key_flag == ON)			motor_key_state(ON);			//开锁
					if(motor_state_key_flag == OFF)			motor_key_state(OFF);			//开锁				
					if(motor_state_key_flag == ERROR)		motor_key_state(ERROR);	//开锁错误				
					motor_time_flag=0;
				}				
			}	
			
			if(motor_state_key_flag == STOP)
			{
				if(nrf_gpio_pin_read(Holzer2) == 0 && nrf_gpio_pin_read(Holzer1) == 0)		//上锁状态  
				{
					KEY_OPEN_dleay_flag=1;						//延迟				KEY_OPEN_flag=1;
					if(KEY_OPEN_flag==1)	//标志位有效
					{							
						LOCK_status_use=1;								//锁以上锁
						if(nrf_gpio_pin_read(KEY_OPEN) == 1 && LOCK_status_flag==0)					//绳索被减断 锁未坏
						{
							DEEP_OPEN_dleay_flag=1;						//延迟 蜂鸣器响 DEEP_OPEN_dleay_flag=0;
							LOCK_status_flag=1;								//锁不能用
						}
					}	
				}	
				else
				{
					KEY_OPEN_dleay_flag=0;						//未上锁 清0 
					KEY_OPEN_flag=0;
					KEY_OPEN_dleay_one_flag=0;				//上锁后只能进一次延迟
				}			
			}
			power_manage(); 
    }
}

/**
 * @}
 */
