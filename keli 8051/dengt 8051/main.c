#include <regx52.h> // AT89C52
void delay(int n)
{
	int i,j;
	for(i=0;i<n;i++)
	{
		for(j=0;j<123;j++);
	}
}
int time;
#define SH P2_0
#define DS P2_1
#define ST P2_2
#define CL1 P2_3
#define CL2 P2_4
#define SH1 P3_0
#define DS1 P3_1
#define ST1 P3_2
#define CL11 P3_5
#define CL22 P3_4
#define x1 P1_0
#define v1 P1_1
#define d1 P1_2
#define x2 P1_3
#define v2 P1_4
#define d2 P1_5
#define btn1 P3_3
#define btn2 P2_6
unsigned char codeled7[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
void IC74595(unsigned char b) {
    unsigned char j;
    for (j = 0; j < 8; j++) {
        DS = (b & (0x80 >> j)) ? 1 : 0; 
        SH = 0;
        SH = 1;
    }
		ST=0;
		ST=1;
}
void IC745951(unsigned char b) {
    unsigned char j;
    for (j = 0; j < 8; j++) {
        DS1 = (b & (0x80 >> j)) ? 1 : 0; 
        SH1 = 0;
        SH1 = 1;
    }
		ST1=0;
		ST1=1;
}
int timetruc1, timetruc2;
int tx1=20,tx2=20;
int tv1=5, tv2=5;
void x1sang()
{
	x1=1;
	v1=0;
	d1=0;
}
void v1sang()
{
	v1=1;
	x1=0;
	d1=0;
}
void d1sang()
{
	d1=1;
	x1=0;
	v1=0;
}
void x2sang()
{
	x2=1;
	d2=0;
	v2=0;
}
void v2sang()
{
	v2=1;
	x2=0;
	d2=0;
}
void d2sang()
{
	d2=1;
	x2=0;
	v2=0;
}
void controlDen()
{
	if(time<=tx1)
	{
		x1sang();
		d2sang();
	}
	if(time>=tx1&&time<tx1+tv1)
	{
		v1sang();
		d2sang();
	}
	if(time>=tx1+tv1&&time<tx1+tv1+tx2)
	{
		d1sang();
		x2sang();
		
	}
	if(time>=tx1+tv1+tx2&&time<tx1+tx2+tv1+tv2)
	{
		d1sang();
		v2sang();
	}
}
	void controltruc()
	{
		if(time==0)
		{
			timetruc1=tx1;
			timetruc2=tx1+tv1;
		}
		if(time==tx1)
		{
			timetruc1=tv1;
			timetruc2=tv1;
		}
		if(time==tx1+tv1)
		{
			timetruc1=tx2+tv2;
			timetruc2=tx2;
		}
		if(time==tx1+tv1+tx2)
		{
			timetruc1=tv2;
			timetruc2=tv2;
		}
		if(time==tx1+tx2+tv1+tv2)
		{
			time=0;
		}
	}
	void hienthiled1(int time1, int time2)
	{
		CL11=0;
		IC745951(codeled7[time1]);
		delay(8);
		CL11=1;
		IC745951(codeled7[time2]);
		CL22=0;
		delay(8);
		CL22=1;
	}
	void hienthiled2(int time1, int time2)
	{
		CL1=0;
		IC74595(codeled7[time1]);
		delay(8);
		CL1=1;
		IC74595(codeled7[time2]);
		CL2=0;
		delay(8);
		CL2=1;
	}
	void controlLED() {
    int i;
    for (i = 0; i < 28; i++) {
        hienthiled1(timetruc1 / 10, timetruc1 % 10);
        hienthiled2(timetruc2 / 10, timetruc2 % 10);
    }
}
	void modeauto()
	{
		
		controlDen();
		controltruc();
		controlLED();
		
	if (time==0) {
		controltruc();
    }
		time++;
		timetruc1--;
		timetruc2--;
	}
	int Mode;
	int check=0;
	int docnut()
	{
		
		if(btn2==0&&check==0)
		{
			Mode=1;
			check=1;
			delay(50);
			return 2;
		}
		return 0;
	}	
	void modeoff()
	{
		CL1=1;
		CL11=1;
		CL22=1;
		CL2=1;
		d1=0;
		x1=0;
		v1=0;
		x2=0;
		v2=0;
		d2=0;
	}
void main() {
	EA=1; //cho phep ngat ngoai toan cuc
	 EX1=1;
 PX1=1; //uu tien ngat INT1
 IT1=1; // NGAT NGOAI 1 CÃNH XUONG
	while(1)
	{
		if(docnut()==2)
		{
			time=0;
		}
		switch(Mode)
		{
			case 0:
			{
				modeoff();
				break;
			}
			case 1:
			{
				
				modeauto();
				break;
			}
			default:
			{
				modeoff();
			}
		}
	}		
 }
void ngatngoai() interrupt 2
{
   Mode=0;
check=0;
}
