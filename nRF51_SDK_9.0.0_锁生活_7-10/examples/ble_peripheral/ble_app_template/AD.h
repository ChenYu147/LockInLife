#ifndef __AD_H
#define	__AD_H

#include "nrf51.h"

void adc_enable(void);
void adc_disable(void);
uint16_t nrf_adc_read(void);

#endif /* __AD_H */
