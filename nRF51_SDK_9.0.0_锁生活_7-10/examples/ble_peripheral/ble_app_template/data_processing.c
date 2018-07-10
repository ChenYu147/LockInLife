#include <string.h>
#include "data_processing.h"
#include "aes.h"
#include "base64.h"
#include "ble_wechat_service.h"

extern receive_data_info receive_data;
extern uint8_t session_key[16];
static ble_wechat_t m_ble_wechat;

//********************************************************//
//解密收到的数据
//*pPlainText: 输出的明文。
//CipherText : 接收到的数据结构体。
//********************************************************//
void receive_data_Decrypt(unsigned char *pPlainText, receive_data_info CipherText)	
{
	CipherText.len=CipherText.len-2;
	char base64_num[(CipherText.len/4)*3];						//算BASE64解密后的数据长度
	int x=0,y=CipherText.len;														
	while(CipherText.data[y-1]==0x3D)y--,x++;					//算多少个‘=’ 
	CipherText.len=CipherText.len/4*3-x;							//求base64解密出来的有效数据的个数
	Base64decode(base64_num,(char *)CipherText.data);	//Base64 解密
	AES_Init(session_key);																
	AES_Decrypt(pPlainText,(uint8_t *)base64_num,CipherText.len,NULL);	//AES 解密
}

void Protocol_analysis(unsigned char *receiveText,char *sendText,int len)	//有问题
{
	char send_data[50];
	send_data_Encrypt(sendText,send_data,len);		
	ble_wechat_indicate_data(&m_ble_wechat,(uint8_t *)send_data,send_data[1]);	
}
//********************************************************//
//加密要发送的数据
//*pPlainText: 输入的明文。
//CipherText : 加密后的数组。
//len 			 : 明文的长度
//********************************************************//
void send_data_Encrypt(char *pPlainText,char *pCipherText,int len)
{
	char AES_num[(len/16+1)*16];					
	AES_Init(session_key);
	AES_Encrypt_PKCS7((unsigned char *)pPlainText,(unsigned char *)AES_num,len,NULL);
	len=(len/16+1)*16;
	Base64encode(&pCipherText[2],AES_num,len);
	pCipherText[0]=0x7e;
	if(len%3 == 0) len = len/3*4;
	else len = (len/3+1)*4;
	pCipherText[1]=len+2;	
}

void get_password(uint8_t *phone_num,uint8_t *time_num,uint8_t *MAC_num,uint8_t *password)
{
	for(int x=0;x<6;x++)
	{
		password[x]=(phone_num[5+x]+time_num[7+x]+(MAC_num[x]%10))%3+1;		
	}	
}
void get_super_password(uint8_t *time_num,uint8_t *MAC_num,uint8_t *password)
{
	for(int x=0;x<6;x++)
	{
		password[x]=(time_num[4+x]+(MAC_num[x]%10))%3+1;		
	}	
}
