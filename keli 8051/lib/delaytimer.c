void delaytimer(int t)
{
	TMOD&=0XF0; //set 4 bit thap cho timer0 va giu nguyen 4 bit cao
	TMOD|=0X01;  // set mode 1 cho timer0 M0=1 VA M1=0
	do{
	TL0=0x18;
	TH0=0xFC;
	TR0=1;
	while(!TF0);
	TR0=0;
	TF0=0;
	t--;
	}while(t>0);
}