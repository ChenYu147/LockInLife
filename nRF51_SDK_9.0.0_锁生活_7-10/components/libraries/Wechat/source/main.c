/**
*
* author: yaobohua
* date: 2014/8/26
* function: ΢������BLE����ͨ�ÿ��
*	modified  by anqiren  2014/12/02  V1.0bat
*
**/
#include "comsource.h"
#include "crc32.h"
#include <string.h>
#include "base64.h"
#include "AES.h"
#include "communication.h"

extern ble_wechat_t m_ble_wechat;
extern data_handler *m_mpbledemo2_handler;
unsigned char MAC_CS[16]="0000123456789abc";
unsigned char num_plain[]="0x11";
char num_AES[]={0};
unsigned char num_aaa[20]={0};

int main(void) {
	
	/** ����32M�ⲿ���� **/
//	NRF_CLOCK->XTALFREQ=0x00;
//	NRF_UICR->XTALFREQ=0x00;
	resource_init();
	advertising_start();
	get_AES_KEY(MAC_CS);
	for(;;) 
	{
		send_data_Encrypt(num_plain,num_AES,MAC_CS,sizeof(num_plain)/sizeof(num_plain[0])-1);		//���������ǲ�֪�����ȵ�
		receive_data_Decrypt(num_AES,num_aaa,MAC_CS,24);			//���룬������֪�����ȵġ�
		wait_app_event();//��ϵͳ���õ��͹���״̬�����ȴ�ϵͳ�¼���
		m_mpbledemo2_handler->m_data_main_process_func(&m_ble_wechat);//ͨ���ӿڵ����豸�����̺�������ѡ��
//�豸��ʹ�õķ�������ѡ�� wechat ���� 
	}
}
