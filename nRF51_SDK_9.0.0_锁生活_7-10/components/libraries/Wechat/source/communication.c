#include "communication.h"
#include "base64.h"
#include "AES.h"
#include <stdio.h>
#include "ble_gap.h"
#include <string.h>

const void *pKey;
extern uint8_t	m_addl_adv_manuf_data[BLE_GAP_ADDR_LEN];
		
char temp[2];		
void get_AES_KEY(unsigned char *pKey)					//�õ�AES����Կ 0000+MAC��ַ
{
//	unsigned char MAC_KEY[16];					//������Կ ����Ϊ16λ
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

void send_data_Encrypt(const unsigned char *in_buf,char *out_buf,void *pKey,int len)	//��AES���ܣ���Base64����
{	
	len=((len/16)+1)*16;
	unsigned char base64_num[len];
	AES_Init(pKey);	
	AES_Encrypt_PKCS7(in_buf,base64_num,len,NULL);	//AES ����
	Base64encode(out_buf,base64_num,len);					//Base64 ����
}

void receive_data_Decrypt(const char *in_buf,unsigned char *out_buf,void *pKey,int len)//��Base64���ܣ���AES����
{
	unsigned char base64_num[(len/4)*3];
	int x=0,y=len;
	while(in_buf[y-1]==0x3D)y--,x++;		//����ٸ���=�� 
	len=len/4*3-x;											//��base64���ܳ�����HEX����
	Base64decode(base64_num , in_buf);	//Base64 ����
	AES_Init(pKey);	
	AES_Decrypt(out_buf,base64_num,len,NULL);	//AES ����
}

void get_password(uint8_t *phone_num,uint8_t *time_num,uint8_t *MAC_num,uint8_t *password)
{
	for(int x=0;x<6;x++)
	{
		password[x]=(phone_num[5+x]+time_num[4+x]+(MAC_num[x]%10))%3+1;		
	}	
}
