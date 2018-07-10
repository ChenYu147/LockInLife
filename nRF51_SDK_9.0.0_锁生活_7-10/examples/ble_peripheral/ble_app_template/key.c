#include "key.h"
#include "nrf_gpio.h"
#include "main.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void key_init(void)
{
	nrf_gpio_cfg_sense_input(KEY1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);					//����1 ���û���
	nrf_gpio_cfg_sense_input(KEY2, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);					//����2 ���û���
	nrf_gpio_cfg_sense_input(KEY3, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);					//����3 ���û���
	nrf_gpio_cfg_sense_input(KEY_OK, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);				//OK�� ���û���
	nrf_gpio_cfg_sense_input(KEY_OPEN , NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);	//�����ƻ� ���û���
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
	if(motor_state_key_flag == STOP)																			//�����������ʱ��
	{
		if(nrf_gpio_pin_read(KEY_OK)==0)																	//OK������
		{	
			if(key_count==6)
			{
				if(compare_password(read_password,password_num) == ON)							//�ж�������ȷ
				{
					motor_state_key_flag=ON;																			//��������	
					memset(password_num,0xFF,8);	//��0
					receive_password_num_update(password_num);
					read_time_flag=0;
					printf("\n The key is unlocked successfully, waiting for the lock \n");
				}		
				else if(compare_password(read_password,password_num) == ERROR)			//�ж��������
				{
					motor_state_key_flag = ERROR;																	//������������			
					printf("\n Key unlock failed. \n");
				}
				else	
					motor_state_key_flag = STOP;												//����������	
			}
			else 	
			{
				printf("\n Key unlock failed. \n");
				motor_state_key_flag = ERROR;																		//������������	
			}		
			key_count=0;																										//��������								
		}		
		
		if (key_scan(read_password,key_count) == 1)										//����а�������
		{	
			printf("\n The key is :%d \n",read_password[key_count]);
			key_count++;		
			if(key_count>7)	key_count=7;
		}

		if(key_flag==1)																						//�а�������
		{
																															//˯��ʱ����0
			key_reset_num=0;																				//��������ʱ����0
			key_reset_flag=0;																				//ȡ�����ð���
			nrf_gpio_pin_clear(LED_BLUE);														//������
			nrf_gpio_pin_set(BEEP);
			nrf_gpio_pin_set(LED_RED);															//��Ʋ���		
		}
		else																											//��������
		{
			nrf_gpio_pin_set(LED_BLUE);															//���Ʋ���
			nrf_gpio_pin_set(LED_RED);															//��Ʋ���	
			nrf_gpio_pin_clear(BEEP);
			if(key_count!=0)
			{
				key_reset_num++;
				if(key_reset_num>80)																	//���100MS*80û���� 
					key_reset_flag=1;																		//���ð�����־
			}
			if(key_reset_flag==1)
			{
				nrf_gpio_pin_clear(LED_RED);													//�����
				nrf_gpio_pin_set(LED_BLUE);														//���Ʋ���
				nrf_gpio_pin_set(BEEP);
				key_count=0;
				key_reset_num++;
				if(key_reset_num>90)
				{
					key_reset_num=0;
					nrf_gpio_pin_set(LED_RED);													//��Ʋ���
					nrf_gpio_pin_set(LED_BLUE);													//���Ʋ���
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
	if(motor_state_key_flag == STOP)																			//�����������ʱ��
	{
		if(nrf_gpio_pin_read(KEY_OK)==0)																	//OK������
		{	
			if(super_key_count==6)
			{
				if(compare_password(read_super_password,super_password_num) == ON)							//�ж�������ȷ
				{
					motor_state_key_flag=ON;																			//��������	
					receive_data_13_flag = 0;
					read_time_flag=0;
					printf("\n The super key is unlocked successfully, waiting for the lock \n");
				}		
				else if(compare_password(read_super_password,super_password_num) == ERROR)			//�ж��������
				{
					motor_state_key_flag = ERROR;																	//������������			
					printf("\n super Key unlock failed. \n");
				}
				else	
					motor_state_key_flag = STOP;												//����������	
			}
			else 	
			{
				printf("\n super Key unlock failed. \n");
				motor_state_key_flag = ERROR;																		//������������	
			}		
			super_key_count=0;																										//��������								
		}		
		
		if (key_scan(read_super_password,super_key_count) == 1)										//����а�������
		{	
			printf("\n The super key is :%d \n",read_super_password[super_key_count]);
			super_key_count++;		
			if(super_key_count>7)	super_key_count=7;
		}

		if(key_flag==1)																						//�а�������
		{
																															//˯��ʱ����0
			super_key_reset_num=0;																				//��������ʱ����0
			super_key_reset_flag=0;																				//ȡ�����ð���
			nrf_gpio_pin_clear(LED_BLUE);														//������
			nrf_gpio_pin_set(BEEP);
			nrf_gpio_pin_set(LED_RED);															//��Ʋ���		
		}
		else																											//��������
		{
			nrf_gpio_pin_set(LED_BLUE);															//���Ʋ���
			nrf_gpio_pin_set(LED_RED);															//��Ʋ���	
			nrf_gpio_pin_clear(BEEP);
			if(key_count!=0)
			{
				super_key_reset_num++;
				if(super_key_reset_num>80)																	//���60MS*80û���� 
					super_key_reset_flag=1;																		//���ð�����־
			}
			if(super_key_reset_flag==1)
			{
				nrf_gpio_pin_clear(LED_RED);													//�����
				nrf_gpio_pin_set(LED_BLUE);														//���Ʋ���
				nrf_gpio_pin_set(BEEP);
				super_key_count=0;
				super_key_reset_num++;
				if(super_key_reset_num>90)
				{
					super_key_reset_num=0;
					nrf_gpio_pin_set(LED_RED);													//��Ʋ���
					nrf_gpio_pin_set(LED_BLUE);													//���Ʋ���
					nrf_gpio_pin_clear(BEEP);
					super_key_reset_flag=0;																					
				}
			}				
		}	
	}	
}
