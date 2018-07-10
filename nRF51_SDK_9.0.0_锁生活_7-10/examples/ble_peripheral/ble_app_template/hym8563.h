#ifndef  HYM8563
#define  HYM8563
#include "nrf_gpio.h"

//******************************************************8563 iic
//#define delay5us {_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();}
//#define delay5us {_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();  }

//unsigned char code c8563_Store[4]={0x08,0x59,0x07,0x01};//д��ʱ���ֵ����һ07:59:00
//д��ʱ���ֵ����һ11:39:00 07 01 01

void iic_start(void);				//����IIC�����ӳ���
void iic_stop(void);				//ֹͣIIC�������ݴ����ӳ���
void slave_NOACK(void);				//�ӻ����ͷ�Ӧ��λ�ӳ�����ʹ���ݴ�����̽���//////////////////////////////////////////////////////
void check_ACK(void);				//����Ӧ��λ����ӳ�����ʹ���ݴ�����̽���
void IICSendByte(unsigned char ch);		//����һ���ֽ�///////////////////////////////////////////////
unsigned char IICreceiveByte(void);		//����һ���ֽ�
void write_CFGNbyte(unsigned char CFG_add,unsigned char *CFG_data2)	;	//����Nλ�Ĵ��������ӳ���
unsigned char receive_CFGbyte(unsigned char  CFG_add);		//����ĳ���Ĵ��������ӳ���
void receive_CFGNbyte(unsigned char *buff);		//����N���Ĵ��������ӳ���
void P8563_Readtime(unsigned char *g8563_Store);
void P8563_settime(unsigned char *g8563_Store);				//дʱ���޸�ֵ
void readtime(unsigned char *timea);
int bcd2hex(int val);
int IsLeapYear(uint16_t year);

int DaysOfOneYear(uint16_t year,unsigned char mon,unsigned char day) ;
int SDateToAbsDays(uint16_t year,unsigned char mon,unsigned char day);
unsigned char  checkTimeValide(unsigned char *startTime,unsigned char *stopTime,unsigned char *currentTime,unsigned char len);
#endif
