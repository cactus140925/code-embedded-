
void IC74595(unsigned char *p, unsigned char n) {
    unsigned char i, j, b;

    for(i = 0; i < n; i++) {
        b = *(p +n-i-1);
        for(j = 0; j < 8; j++) {
            IC_DS = b & (0x80 >> j);
            IC_SH = 0;
            IC_SH = 1;
        }
		P0^i=0;
		IC_ST = 0;  
    IC_ST = 1;
				delay(100);
			P0^i=1;
    }
}
