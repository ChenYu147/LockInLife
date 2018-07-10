
#include "hym8563.h"

#define MIN 0x02
#define SEC 0x03
#define HOUR 0x04
#define DAY 0x05
#define WEEK 0x06
#define MONTH 0x07
#define YEAR 0x08
#define read_ADD 0xa3		//写器件地址
#define write_ADD 0xa2		//读器件地址
#define delay5us delaylong(10);
#define SCL(status) {nrf_gpio_cfg_output(25); setPin(25,status); }
#define SDA(inout,status) {if(inout)nrf_gpio_cfg_output(24);else nrf_gpio_cfg_input(24,NRF_GPIO_PIN_NOPULL);setPin(24,status);}//时钟数据
#define SDAIN  {nrf_gpio_cfg_input(24,NRF_GPIO_PIN_NOPULL);}
#define SDAT nrf_gpio_pin_read(24)
//sbit SCL=P0^7;//时钟的时钟
uint8_t F0;
uint8_t  c8563_Store[7]={0x4,0x6,0x5,0x02,0x01,0x01,0x07};
uint8_t  SystemError;				//从机错误标志位

void setPin(uint8_t ionum,uint8_t leve)
{
		if(leve)
		{
				nrf_gpio_pin_set(ionum);
		}
		else
		{
				nrf_gpio_pin_clear(ionum);
		}
			
}

void delaylong (uint16_t time)	//0.5us
{
		while(time--);
}

void iic_start(void)				//启动IIC总线子程序
{
	delay5us;
	//SDA=1;
	SDA(0X8,1);
	SCL(1);
	delay5us;
	SDA(0X8,0);
	delay5us;
	SCL(0);
}

void iic_stop(void)				//停止IIC总线数据传送子程序
{
	SDA(0X8,0);
	SCL(1);
	delay5us;
	SDA(0X8,1);
	delay5us;
	SCL(0);
	delay5us;
}
void slave_NOACK(void)				//从机发送非应答位子程序，迫使数据传输过程结束//////////////////////////////////////////////////////
{
	SDA(0X8,1);
	SCL(1);
	delay5us;
	SCL(0);
	SDA(0X8,0);

}

void check_ACK(void)				//主机应答位检查子程序，迫使数据传输过程结束
{
	SDA(0X8,1);
	SCL(1);
	F0=0;
	SDAIN;
	if(SDAT==1) F0=1;				//惹SDA=1表明非应答，置位非应答标志F0
	SCL(0);
}

void IICSendByte(uint8_t ch)		//发送一个字节///////////////////////////////////////////////
{
	uint8_t n=8;
	while(n--)
	{
		if((ch&0x80)==0x80)
		{
			SDA(0X8,1);
			SCL(1);
		    delay5us;
			SCL(0);		
			SDA(0X8,0);
		}
		else
		{
			SDA(0X8,0);
			SCL(1);
			delay5us;
			SCL(0);
		}
		ch=ch<<1;
	}
}

uint8_t IICreceiveByte(void)		//接收一个字节
{
	uint8_t  n=8;
	uint8_t tdata;
	while(n--)
	{
		SDA(0X8,1);
	delay5us;
		SCL(1);
		tdata=tdata<<1;
		SDAIN;
		if(SDAT==1)
			tdata=tdata|0x01;
		else
			tdata=tdata&0xfe;
		SCL(0);
	delay5us;
		}
	return(tdata);
}
void write_CFGNbyte(uint8_t CFG_add,uint8_t *CFG_data2)		//发送N位寄存器数据子程序
{
	uint8_t i;
  //  _disable();
	iic_start();
	IICSendByte(write_ADD);
	check_ACK();
	if(F0==1)
	{
		SystemError=1;
		return;
	}
	IICSendByte(CFG_add);
	check_ACK();
	if(F0==1)
	{
		SystemError=1;
		return;
	}
	for(i=0;i<7;i++)
	{
	   IICSendByte(CFG_data2[i]);
	   check_ACK();
	   if(F0==1)
		{
		   SystemError=1;
		   return;
		}
	}
	iic_stop();
	//_enable();;
}


uint8_t receive_CFGbyte(uint8_t  CFG_add)		//接收某个寄存器数据子程序
{
	uint8_t  receive_da;
//	_disable();
	iic_start();
	IICSendByte(write_ADD);
	check_ACK();
	if(F0==1)
	{
		SystemError=1;
		return(0);
	}
	IICSendByte(CFG_add);
	check_ACK();
	if(F0==1)
	{
		SystemError=1;
		return(0);
	}
	iic_start();
	IICSendByte(read_ADD);
	check_ACK();/////////////////////////
	if(F0==1)
	{
		SystemError=1;
		return(0);
	}

	receive_da=IICreceiveByte();
	slave_NOACK();
	iic_stop();
	//_enable();;
	return(receive_da);
	
}
void receive_CFGNbyte(uint8_t *buff)		//接收N个寄存器数据子程序

{
//	_disable();
		buff[6]=receive_CFGbyte(0x08);
		buff[5]=receive_CFGbyte(0x07);
		buff[4]=receive_CFGbyte(0x06);
		buff[3]=receive_CFGbyte(0x05);
		buff[2]=receive_CFGbyte(0x04);
		buff[1]=receive_CFGbyte(0x03);
		buff[0]=receive_CFGbyte(0x02);
 //   _enable();;
}
void P8563_Readtime(uint8_t *g8563_Store)
	{
	uint8_t time[7];
	receive_CFGNbyte(time);
	g8563_Store[0]=time[0]&0x7f;
	g8563_Store[1]=time[1]&0x7f;
	g8563_Store[2]=time[2]&0x3f;
	g8563_Store[3]=time[3]&0x3f;
	g8563_Store[4]=time[4]&0x07;
	g8563_Store[5]=time[5]&0x1f;
	g8563_Store[6]=time[6]&0xff;
	}
void P8563_settime(uint8_t*g8563_Store)				//写时间修改值
	{
	write_CFGNbyte(MIN,g8563_Store);
	}
void readtime(uint8_t *timea)
	{
    uint8_t g8563_Store[7];
	P8563_Readtime(g8563_Store);
	timea[0]=g8563_Store[4]+1;  
	timea[1]=g8563_Store[6];
	timea[2]=g8563_Store[5];
	timea[3]=g8563_Store[3];
	timea[4]=g8563_Store[2];
	timea[5]=g8563_Store[1];  
	timea[6]=g8563_Store[0];  
	}

int bcd2hex(int val)
{
	return (val/16*10+val%16);
}
int IsLeapYear(uint16_t year) {
 return (year%4==0&&year%100)||year%400==0;
}

int DaysOfOneYear(uint16_t year,uint8_t mon,uint8_t day) {
 switch(mon-1) {
 case 11:
  day+=30;
 case 10:
  day+=31;
 case 9:
  day+=30;
 case 8:
  day+=31;
 case 7:
  day+=31;
 case 6:
  day+=30;
 case 5:
  day+=31;
 case 4:
  day+=30;
 case 3:
  day+=31;
 case 2:
  day+=IsLeapYear(year)?29:28;
 case 1:
  day+=31;
 }
 return day;
}

int SDateToAbsDays(uint16_t year,uint8_t mon,uint8_t day) {
 int years;
	int days;
	 years = bcd2hex(year) -1;
   days = years*365 + years/4 - years/100 + years/400;
	
 days+=DaysOfOneYear(bcd2hex(year),bcd2hex(mon),bcd2hex(day));
 return days;
}
uint8_t checkTimeValide(uint8_t *startTime,uint8_t *stopTime,uint8_t *currentTime,uint8_t len)
{
    uint8_t i=0,j=0,faildFlag=0,faildFlag2=0;
    for(i=0;i<len;i++)
    {
        if(startTime[i]>currentTime[i])
				{
						faildFlag=1;
						break;
				}
				if(startTime[i]<currentTime[i])
				{
						break;
				}
    } 
    if(i==len||faildFlag==0)
		{
				for(j=0;j<len;j++)
				{
						if(currentTime[j]>stopTime[j])
						{
								faildFlag2=1;
								break;
						}
						if(currentTime[j]<stopTime[j])
						{
								break;
						}
				}
				if(faildFlag2==1/*||j==3*/)
						return 0;
				else
						return 1;
		}
    else
    {
				return 0;
    }       
}
