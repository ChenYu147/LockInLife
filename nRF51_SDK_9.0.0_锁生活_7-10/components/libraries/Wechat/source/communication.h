#ifndef COMMUNICATION_H__
#define COMMUNICATION_H__

void get_AES_KEY(unsigned char *pKey);
void send_data_Encrypt(const unsigned char *in_buf,char *out_buf,void *pKey,int len);	//��AES���ܣ���Base64����
void receive_data_Decrypt(const char *in_buf,unsigned char *out_buf,void *pKey,int len);	
#endif // COMMUNICATION_H__
