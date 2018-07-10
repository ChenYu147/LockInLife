#ifndef __DATA_PROCESSING_H
#define	__DATA_PROCESSING_H

#include <stdint.h>

typedef struct
{	
	uint8_t flag;
	uint8_t len;
	uint8_t *data;
} receive_data_info;
/**接收数据结构体**/

typedef struct
{	
	uint8_t bMagicNumber;
	uint8_t len;
	char *data;
} send_data_info;
/**发送数据结构体**/

void receive_data_Decrypt(unsigned char *pPlainText, receive_data_info CipherText);
void Protocol_analysis(unsigned char *receiveText,char *sendText,int len);
void send_data_Encrypt(char *pPlainText,char *pCipherText,int len);
void get_password(uint8_t *phone_num,uint8_t *time_num,uint8_t *MAC_num,uint8_t *password);
void get_super_password(uint8_t *time_num,uint8_t *MAC_num,uint8_t *password);

#endif /* __DATA_PROCESSING_H */
