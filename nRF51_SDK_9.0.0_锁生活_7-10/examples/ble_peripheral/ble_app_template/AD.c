#include "ad.h"
#include "nrf51.h"
#include "nrf_gpio.h"

void adc_enable()
{
		NRF_ADC->CONFIG = ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos;		//设置为10位ADC
		NRF_ADC->CONFIG |= ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos;//AD输入使用1/3预引比例分子 及输入电压除以3 
		NRF_ADC->CONFIG |= ADC_CONFIG_REFSEL_SupplyOneThirdPrescaling << ADC_CONFIG_REFSEL_Pos;//使用带有1/3预缩放的VDD 电压范围为(0~1/3*VDD)
		NRF_ADC->CONFIG |= (1 << 7) << ADC_CONFIG_PSEL_Pos;//设置输入引脚 AIN5
		NRF_ADC->ENABLE = 1;        
}

void adc_disable()
{
		NRF_ADC->ENABLE = 0;
}

uint16_t nrf_adc_read(void)  //读取数据
{
  uint16_t adc_data;
  NRF_ADC->TASKS_START = 1;					//启动ADC
  while(NRF_ADC->EVENTS_END == 0);	//等待ADC停止
  NRF_ADC->EVENTS_END = 0;					//清零标志位
  adc_data = NRF_ADC->RESULT;				//读取数据
  return adc_data;									//返回数据
}
