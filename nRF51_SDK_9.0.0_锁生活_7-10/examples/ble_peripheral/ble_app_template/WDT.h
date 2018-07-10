#ifndef __WDT_H
#define	__WDT_H

#include "nrf_drv_wdt.h"
#include "app_util_platform.h"

void wdt_init(void);
void wdt_start(void);
void wdt_feed(void);
void wdt_stop(void);
	
#endif /* __WDT_H */
