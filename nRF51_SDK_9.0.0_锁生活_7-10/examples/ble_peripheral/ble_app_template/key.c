#include "key.h"
#include "nrf_gpio.h"
#include "main.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void key_init(void)
{
	nrf_gpio_cfg_sense_input(KEY1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);					//数字1 设置唤醒
	nrf_gpio_cfg_sense_input(KEY2, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);					//数字2 设置唤醒
	nrf_gpio_cfg_sense_input(KEY3, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);					//数字3 设置唤醒
	nrf_gpio_cfg_sense_input(KEY_OK, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);				//OK键 设置唤醒
	nrf_gpio_cfg_sense_input(KEY_OPEN , NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);	//暴力破坏 设置唤醒
}

bool key_flag=0;
uint8_t motor_state_key_flag=STOP;
extern uint8_t password_num[6];
extern uint8_t super_password_num[8];

uint8_t key_scan(uint8_t * value,uint8_t count)
{	
	static uint8_t key1_flag=0, key2_flag=0, key3_flag=0;
	
	if(nrf_gpio_pin_read(KEY1)==0 | nrf_gpio_pin_read(KEY2)==0 | nrf_gpio_pin_read(KEY3)==0)
	{
		key_flag=1;
	}
	else key_flag=0;
	
	if(nrf_gpio_pin_read(KEY1)==0 && key1_flag==0)
	{
		key1_flag = 1;
		value[count]=1;
		return 1;	
	}
	if(nrf_gpio_pin_read(KEY1)==1)
	{
		key1_flag = 0;			
	}
	
	if(nrf_gpio_pin_read(KEY2)==0 && key2_flag==0)
	{
		key2_flag=1;
		value[count]=2;
		return 1;	
	}
	if(nrf_gpio_pin_read(KEY2)==1)
	{
		key2_flag = 0;			
	}
	
	if(nrf_gpio_pin_read(KEY3)==0 && key3_flag==0)
	{
		key3_flag=1;
		value[count]=3;
		return 1;		
	}
	if(nrf_gpio_pin_read(KEY3)==1)
	{
		key3_flag = 0;
	}	
	
	return 0;
}

uint8_t compare_password(uint8_t *key_num,uint8_t *compare_password_num)
{	
	for(int x=0;x<6;x++)
	{
		if(compare_password_num[x] != key_num[x])
		{
			return ERROR;
		}
	}
	return ON;
}

uint8_t read_password[10]={0};
uint8_t key_count=0;
uint8_t key_reset_num=0,key_reset_flag=0;
uint8_t read_time_flag=0;
void key_run()
{	
	if(motor_state_key_flag == STOP)																			//电机不动作的时候
	{
		if(nrf_gpio_pin_read(KEY_OK)==0)																	//OK键按下
		{	
			if(key_count==6)
			{
				if(compare_password(read_password,password_num) == ON)							//判断密码正确
				{
					motor_state_key_flag=ON;																			//按键开锁	
					memset(password_num,0xFF,8);	//清0
					receive_password_num_update(password_num);
					read_time_flag=0;
					printf("\n The key is unlocked successfully, waiting for the lock \n");
				}		
				else if(compare_password(read_password,password_num) == ERROR)			//判断密码错误
				{
					motor_state_key_flag = ERROR;																	//按键开锁错误			
					printf("\n Key unlock failed. \n");
				}
				else	
					motor_state_key_flag = STOP;												//按键不开锁	
			}
			else 	
			{
				printf("\n Key unlock failed. \n");
				motor_state_key_flag = ERROR;																		//按键开锁错误	
			}		
			key_count=0;																										//按键清零								
		}		
		
		if (key_scan(read_password,key_count) == 1)										//如果有按键按下
		{	
			printf("\n The key is :%d \n",read_password[key_count]);
			key_count++;		
			if(key_count>7)	key_count=7;
		}

		if(key_flag==1)																						//有按键按下
		{
																															//睡眠时间清0
			key_reset_num=0;																				//按键重置时间清0
			key_reset_flag=0;																				//取消重置按键
			nrf_gpio_pin_clear(LED_BLUE);														//蓝灯亮
			nrf_gpio_pin_set(BEEP);
			nrf_gpio_pin_set(LED_RED);															//红灯不亮		
		}
		else																											//不按按键
		{
			nrf_gpio_pin_set(LED_BLUE);															//蓝灯不亮
			nrf_gpio_pin_set(LED_RED);															//红灯不亮	
			nrf_gpio_pin_clear(BEEP);
			if(key_count!=0)
			{
				key_reset_num++;
				if(key_reset_num>80)																	//如果100MS*80没按下 
					key_reset_flag=1;																		//重置按键标志
			}
			if(key_reset_flag==1)
			{
				nrf_gpio_pin_clear(LED_RED);													//红灯亮
				nrf_gpio_pin_set(LED_BLUE);														//蓝灯不亮
				nrf_gpio_pin_set(BEEP);
				key_count=0;
				key_reset_num++;
				if(key_reset_num>90)
				{
					key_reset_num=0;
					nrf_gpio_pin_set(LED_RED);													//红灯不亮
					nrf_gpio_pin_set(LED_BLUE);													//蓝灯不亮
					nrf_gpio_pin_clear(BEEP);
					key_reset_flag=0;																					
				}
			}				
		}	
	}	
}
uint8_t read_super_password[10]={0};
uint8_t super_key_count=0;
uint8_t super_key_reset_num=0,super_key_reset_flag=0;
uint8_t super_read_time_flag=0;
extern uint8_t receive_data_13_flag;

void key_super_run()
{	
	if(motor_state_key_flag == STOP)																			//电机不动作的时候
	{
		if(nrf_gpio_pin_read(KEY_OK)==0)																	//OK键按下
		{	
			if(super_key_count==6)
			{
				if(compare_password(read_super_password,super_password_num) == ON)							//判断密码正确
				{
					motor_state_key_flag=ON;																			//按键开锁	
					receive_data_13_flag = 0;
					read_time_flag=0;
					printf("\n The super key is unlocked successfully, waiting for the lock \n");
				}		
				else if(compare_password(read_super_password,super_password_num) == ERROR)			//判断密码错误
				{
					motor_state_key_flag = ERROR;																	//按键开锁错误			
					printf("\n super Key unlock failed. \n");
				}
				else	
					motor_state_key_flag = STOP;												//按键不开锁	
			}
			else 	
			{
				printf("\n super Key unlock failed. \n");
				motor_state_key_flag = ERROR;																		//按键开锁错误	
			}		
			super_key_count=0;																										//按键清零								
		}		
		
		if (key_scan(read_super_password,super_key_count) == 1)										//如果有按键按下
		{	
			printf("\n The super key is :%d \n",read_super_password[super_key_count]);
			super_key_count++;		
			if(super_key_count>7)	super_key_count=7;
		}

		if(key_flag==1)																						//有按键按下
		{
																															//睡眠时间清0
			super_key_reset_num=0;																				//按键重置时间清0
			super_key_reset_flag=0;																				//取消重置按键
			nrf_gpio_pin_clear(LED_BLUE);														//蓝灯亮
			nrf_gpio_pin_set(BEEP);
			nrf_gpio_pin_set(LED_RED);															//红灯不亮		
		}
		else																											//不按按键
		{
			nrf_gpio_pin_set(LED_BLUE);															//蓝灯不亮
			nrf_gpio_pin_set(LED_RED);															//红灯不亮	
			nrf_gpio_pin_clear(BEEP);
			if(key_count!=0)
			{
				super_key_reset_num++;
				if(super_key_reset_num>80)																	//如果60MS*80没按下 
					super_key_reset_flag=1;																		//重置按键标志
			}
			if(super_key_reset_flag==1)
			{
				nrf_gpio_pin_clear(LED_RED);													//红灯亮
				nrf_gpio_pin_set(LED_BLUE);														//蓝灯不亮
				nrf_gpio_pin_set(BEEP);
				super_key_count=0;
				super_key_reset_num++;
				if(super_key_reset_num>90)
				{
					super_key_reset_num=0;
					nrf_gpio_pin_set(LED_RED);													//红灯不亮
					nrf_gpio_pin_set(LED_BLUE);													//蓝灯不亮
					nrf_gpio_pin_clear(BEEP);
					super_key_reset_flag=0;																					
				}
			}				
		}	
	}	
}
