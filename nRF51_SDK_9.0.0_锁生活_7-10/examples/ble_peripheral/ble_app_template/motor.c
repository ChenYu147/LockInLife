#include "motor.h"
#include "nrf_gpio.h"
#include "main.h"

void motor_init(void)
{
	nrf_gpio_cfg_output(motor_z);
	nrf_gpio_cfg_output(motor_f);
	nrf_gpio_cfg_output(motor_en);
	
	nrf_gpio_pin_clear(motor_z);
	nrf_gpio_pin_clear(motor_f);	
	nrf_gpio_pin_clear(motor_en);
}

static uint8_t motor_key_on_time_num=0,motor_key_off_time_num=0,motor_key_error_time_num=0;
extern uint8_t motor_state_key_flag;
extern uint8_t receive_data_12_flag;
extern char	motor_delay_flag;
extern int KEY_OPEN_dleay_num;

void motor_key_state(uint8_t state_flag)
{	
	if(state_flag == ON)									//¿ªËø
	{
		motor_key_on_time_num++;	
		if(motor_key_on_time_num<3 || nrf_gpio_pin_read(Holzer2) == 0)	//¹æ¶¨Ê±¼äÄÚ£¬»òÉÏËø×´Ì¬
		{
			if(motor_key_on_time_num<2)
			{
				nrf_gpio_pin_set(BEEP);			
			}
			else if(motor_key_on_time_num<3)
			{
				nrf_gpio_pin_clear(BEEP);			
			}
			nrf_gpio_pin_clear(LED_BLUE);				//À¶µÆÁÁ
			nrf_gpio_pin_set(LED_RED);					//ºìµÆ²»ÁÁ		
			if(nrf_gpio_pin_read(Holzer2) == 0) 	//ÉÏËø×´Ì¬
			{
				nrf_gpio_pin_clear(motor_f);
				nrf_gpio_pin_set(motor_z);	
				nrf_gpio_pin_set(motor_en);
			}
			else	
			{
				nrf_gpio_pin_clear(motor_z);
				nrf_gpio_pin_clear(motor_f);	
				nrf_gpio_pin_clear(motor_en);
			}
			motor_state_key_flag=ON; 						//¿ªËø×´Ì¬ 				
		}				
		else																//Í£×ª
		{	
			if(receive_data_12_flag==1)			//0x12 ¿ªËø
			{
				motor_delay_flag=1;							//µç»úÑÓÊ±		
			}	
			nrf_gpio_pin_set(LED_BLUE);				//À¶µÆ²»ÁÁ
			nrf_gpio_pin_set(LED_RED);				//ºìµÆ²»ÁÁ	
			nrf_gpio_pin_clear(motor_z);
			nrf_gpio_pin_clear(motor_f);	
			nrf_gpio_pin_clear(motor_en);
			printf("\n Motor is open \n");
			motor_key_on_time_num=0;
			motor_state_key_flag=STOP;			//²»¿ªËø×´Ì¬	
			KEY_OPEN_dleay_num=0;						//»·Â·¼ì²âÑÓ³ÙÇå0 
		}		
	}
	
	else if(state_flag == OFF)							//¹ØËø
	{
		motor_key_off_time_num++;		
		if(motor_key_off_time_num>=3)					//Í£×ª
		{			
			nrf_gpio_pin_set(LED_BLUE);				//À¶µÆ²»ÁÁ
			nrf_gpio_pin_set(LED_RED);				//ºìµÆ²»ÁÁ	
			nrf_gpio_pin_clear(motor_z);
			nrf_gpio_pin_clear(motor_f);	
			nrf_gpio_pin_clear(motor_en);
			if(motor_key_off_time_num<4)
			{
				nrf_gpio_pin_set(BEEP);			
			}
			else if(motor_key_off_time_num<5)
			{
				nrf_gpio_pin_clear(BEEP);			
			}
			else
			{
				motor_key_off_time_num=0;
				motor_state_key_flag=STOP;				//²»¿ªËø×´Ì¬		
				printf("\n lock is close \n");					
				KEY_OPEN_dleay_num=0;						//»·Â·¼ì²âÑÓ³ÙÇå0 
			}							
		}
		else	
		{
			nrf_gpio_pin_clear(LED_BLUE);				//À¶µÆÁÁ
			nrf_gpio_pin_set(LED_RED);					//ºìµÆ²»ÁÁ	
			nrf_gpio_pin_clear(motor_z);
			nrf_gpio_pin_set(motor_f);	
			nrf_gpio_pin_set(motor_en);
			if(motor_key_off_time_num<2)
			{
				nrf_gpio_pin_set(BEEP);			
			}
			else if(motor_key_off_time_num<3)
			{
				nrf_gpio_pin_clear(BEEP);			
			}	
			motor_state_key_flag=OFF; 						//¿ªËø×´Ì¬ 
		}
	}
	
	else if(state_flag == ERROR)						//¿ªËø´íÎó
	{		
		motor_key_error_time_num++;
		nrf_gpio_pin_set(LED_BLUE);								//À¶µÆ²»ÁÁ
		nrf_gpio_pin_clear(LED_RED);							//ºìµÆÁÁ	
		nrf_gpio_pin_clear(motor_z);
		nrf_gpio_pin_clear(motor_f);	
		nrf_gpio_pin_clear(motor_en);	
		nrf_gpio_pin_set(BEEP);	
		if(motor_key_error_time_num>3)
		{
			motor_key_error_time_num=0;
			nrf_gpio_pin_set(LED_BLUE);							//À¶µÆ²»ÁÁ
			nrf_gpio_pin_set(LED_RED);							//ºìµÆ²»ÁÁ	
			motor_state_key_flag=STOP;						//²»¿ªËø×´Ì¬
			nrf_gpio_pin_clear(BEEP);	
			printf("\n wrong password \n");	
		}
		else 
			motor_state_key_flag=ERROR;					//¿ªËø´íÎó×´Ì¬
	}
	
	else 		
	{
		nrf_gpio_pin_clear(motor_z);
		nrf_gpio_pin_clear(motor_f);	
		nrf_gpio_pin_clear(motor_en);		
		motor_state_key_flag=STOP;							//²»¿ªËø	×´Ì¬
	}
}
