#include "ad.h"
#include "nrf51.h"
#include "nrf_gpio.h"

void adc_enable()
{
		NRF_ADC->CONFIG = ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos;		//����Ϊ10λADC
		NRF_ADC->CONFIG |= ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos;//AD����ʹ��1/3Ԥ���������� �������ѹ����3 
		NRF_ADC->CONFIG |= ADC_CONFIG_REFSEL_SupplyOneThirdPrescaling << ADC_CONFIG_REFSEL_Pos;//ʹ�ô���1/3Ԥ���ŵ�VDD ��ѹ��ΧΪ(0~1/3*VDD)
		NRF_ADC->CONFIG |= (1 << 7) << ADC_CONFIG_PSEL_Pos;//������������ AIN5
		NRF_ADC->ENABLE = 1;        
}

void adc_disable()
{
		NRF_ADC->ENABLE = 0;
}

uint16_t nrf_adc_read(void)  //��ȡ����
{
  uint16_t adc_data;
  NRF_ADC->TASKS_START = 1;					//����ADC
  while(NRF_ADC->EVENTS_END == 0);	//�ȴ�ADCֹͣ
  NRF_ADC->EVENTS_END = 0;					//�����־λ
  adc_data = NRF_ADC->RESULT;				//��ȡ����
  return adc_data;									//��������
}
