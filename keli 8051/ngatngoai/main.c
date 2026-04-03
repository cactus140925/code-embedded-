#include<regx52.h>
#include"../lib/delay.h"
sbit LED1 =P1^0;
sbit LED2 = P1^1;
 
 void main()
 {
	 EA=1; //cho phep ngat ngoai toan cuc
	 EX1=1;
 PX1=1; //uu tien ngat INT1
 IT1=1; // NGAT NGOAI 1 CÃNH XUONG
	 while(1)
	 {
		 LED1=0;
		 delay(500);
		 LED1=1;
		 delay(500);
	 }
 }
 void ngatngoai() interrupt 2
 {
	 LED2=~LED2;
 }
	 
		 
 