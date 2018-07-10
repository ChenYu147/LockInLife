#ifndef  HYM8563
#define  HYM8563
#include "nrf_gpio.h"

//******************************************************8563 iic
//#define delay5us {_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();}
//#define delay5us {_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();  }

//unsigned char code c8563_Store[4]={0x08,0x59,0x07,0x01};//写入时间初值，周一07:59:00
//写入时间初值，周一11:39:00 07 01 01

void iic_start(void);				//启动IIC总线子程序
void iic_stop(void);				//停止IIC总线数据传送子程序
void slave_NOACK(void);				//从机发送非应答位子程序，迫使数据传输过程结束//////////////////////////////////////////////////////
void check_ACK(void);				//主机应答位检查子程序，迫使数据传输过程结束
void IICSendByte(unsigned char ch);		//发送一个字节///////////////////////////////////////////////
unsigned char IICreceiveByte(void);		//接收一个字节
void write_CFGNbyte(unsigned char CFG_add,unsigned char *CFG_data2)	;	//发送N位寄存器数据子程序
unsigned char receive_CFGbyte(unsigned char  CFG_add);		//接收某个寄存器数据子程序
void receive_CFGNbyte(unsigned char *buff);		//接收N个寄存器数据子程序
void P8563_Readtime(unsigned char *g8563_Store);
void P8563_settime(unsigned char *g8563_Store);				//写时间修改值
void readtime(unsigned char *timea);
int bcd2hex(int val);
int IsLeapYear(uint16_t year);

int DaysOfOneYear(uint16_t year,unsigned char mon,unsigned char day) ;
int SDateToAbsDays(uint16_t year,unsigned char mon,unsigned char day);
unsigned char  checkTimeValide(unsigned char *startTime,unsigned char *stopTime,unsigned char *currentTime,unsigned char len);
#endif
