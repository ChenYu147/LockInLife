#include "wdt.h"

#define RELOAD_COUNT (32768*10-1)																			//10s

void wdt_init(void)    
{    
	NRF_WDT->TASKS_START = 0;   																				//不启动看门狗 
	NRF_WDT->CRV = RELOAD_COUNT;    																		//计数值
	NRF_WDT->CONFIG =    
	WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos |    
	WDT_CONFIG_SLEEP_Pause << WDT_CONFIG_SLEEP_Pos;  
	//调试时 停止 看门狗
	//休眠时 停止 看门狗
	NRF_WDT->RREN = WDT_RREN_RR0_Enabled << WDT_RREN_RR0_Pos;   				//使能 RR0
	NRF_WDT->INTENSET = WDT_INTENSET_TIMEOUT_Msk;  											//使能中断
	NVIC_ClearPendingIRQ(WDT_IRQn);   																	
	NVIC_SetPriority(WDT_IRQn, APP_IRQ_PRIORITY_LOW);  									
	NVIC_EnableIRQ(WDT_IRQn);  
}   
void wdt_start(void)  
{  
    NRF_WDT->TASKS_START = 1;  
}   
void wdt_feed(void)  
{  
    if(NRF_WDT->RUNSTATUS & WDT_RUNSTATUS_RUNSTATUS_Msk)  
        NRF_WDT->RR[0] = WDT_RR_RR_Reload;  
}    
void wdt_stop(void)  
{  
    NRF_WDT->TASKS_START = 0;  
} 


void wdt_event_handler(void)
{
	//LEDS_OFF(LEDS_MASK);
	NVIC_SystemReset();								//重启
	//NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}
