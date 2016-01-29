#include <stdint.h>
#include <stdio.h>
//
#include <CHANTILLY_BP.h>
//
typedef float float_t;
#define _BB_BOARD	0x07
#define _IO24_BOARD	0x01
// x10 command brief
#define _IMMEDIATE				0x01
#define _SERVO_MODE_RUN			0x05
#define _STOP					0x06
#define _SET_D_OUTPUT			0x0a
#define _SET_D_NORMAL			0x0b
#define _BURST_OUTPUT_EXECUTE	0x0c
//
#define _EXTVIN_ADJUST  0.5 /* resitor tolerance : user's adjustment if needed */
#define _BB_ADC_ALIASING        (float) 5.0/1024.0
#define _BB_BATTERY_VOLTS_AS_FLOAT(x) (float_t) ((float_t) ((*x | (*(x+1)<<8)) * (_BB_ADC_ALIASING)) / (0.5 ))
#define _BB_BATTERY_FLOAT_AS_UINT16(x) (uint16_t) ( ( x / 2.0 ) / (5.0/1024))
//
#define _BB_MAIN_VOLTS_AS_FLOAT(x) (float_t) ((float_t) ((*x | (*(x+1)<<8)) * (_BB_ADC_ALIASING)) / (4.7/(33.0+4.7+_EXTVIN_ADJUST))) 
#define _BB_HOST_3V3_AS_FLOAT(x)     (float_t) ((float_t) ((*x | (*(x+1)<<8)) * (_BB_ADC_ALIASING)) / ( 1.0 ))
//
void burst_do_command(uint8_t nibble_count,uint8_t data,uint8_t delay)
{
	// special data prepare for lcd 4 bit output bursts
	uint8_t ttx[64];
	uint8_t hi_n,lo_n;
	uint8_t rev=0;
	const uint8_t cp[8]={1,2,4,8,16,32,64,128};
	//
	printf("REV:");
	if(data&cp[0]) {rev|=128;printf("1");}else{printf("0");}
	if(data&cp[1]) {rev|=64;printf("1");}else{printf("0");}
	if(data&cp[2]) {rev|=32;printf("1");}else{printf("0");}
	if(data&cp[3]) {rev|=16;printf("1");}else{printf("0");}
	if(data&cp[4]) {rev|=8;printf("1");}else{printf("0");}
	if(data&cp[5]) {rev|=4;printf("1");}else{printf("0");}
	if(data&cp[6]) {rev|=2;printf("1");}else{printf("0");}
	if(data&cp[7]) {rev|=1;printf("1");}else{printf("0");}
	printf("\n");
		// end reverse
		hi_n=rev&0b1111;	// yep, inverted
		lo_n=rev>>4;		// same here
		printf("command burst (Hi =%01x , lo=%01x",hi_n,lo_n);
	
		ttx[0]=0 | ((hi_n)<<4) | 0<<3 | 0<<2;	// e low, Register mode
		ttx[1]=0 | ((hi_n)<<4) | 1<<3 | 0<<2;	// e hig, Register mode
		ttx[2]=0 | ((hi_n)<<4) | 0<<3 | 0<<2;	// e low, Register mode
		// end hinibble,
		// low nibble :
		ttx[3]=0 | ((lo_n)<<4) | 0<<3 | 0<<2;	// e low, Register mode
		ttx[4]=0 | ((lo_n)<<4) | 1<<3 | 0<<2;	// e hig, Register mode
		ttx[5]=0 | ((lo_n)<<4) | 0<<3 | 0<<2;	// e low, Register mode
		// end built
		// prepare delays
		ttx[16+0]=delay;
		ttx[16+1]=delay;
		ttx[16+2]=delay;
		ttx[16+3]=delay;
		ttx[16+4]=delay;
		ttx[16+5]=delay;
		//ttx[16+6]=delay;
		// send
		// prepare addr
		CHAN_addr(_IO24_BOARD,0x160);
		CHAN_setBytes(_IO24_BOARD,32,ttx);	// 32 bytes at a glance , delay compris
		//
		CHAN_addr(_IO24_BOARD,0x154);
		// sets quantity to output according to nibble count. nibble=1 : hi nibble only outputed
		ttx[0]=nibble_count*3;//6;									// we want 8 steps output to be played
		ttx[1]=0;
		CHAN_setBytes(_IO24_BOARD,2,ttx);	
		//
		// execute burst
		printf("COMMAND : burst output to D\n");
		CHAN_command(_IO24_BOARD,_BURST_OUTPUT_EXECUTE);
}
//
void burst_do_ddram(uint8_t data,uint8_t delay)
{
	// special data prepare for lcd 4 bit output bursts
	uint8_t ttx[64];
	uint8_t hi_n,lo_n;
	uint8_t rev=0;
	const uint8_t cp[8]={1,2,4,8,16,32,64,128};
	if(data&cp[0]) rev|=128;
	if(data&cp[1]) rev|=64;
	if(data&cp[2]) rev|=32;
	if(data&cp[3]) rev|=16;
	if(data&cp[4]) rev|=8;
	if(data&cp[5]) rev|=4;
	if(data&cp[6]) rev|=2;
	if(data&cp[7]) rev|=1;
	
		// end reverse
		// split nibbles
		hi_n=rev&0b1111;	// yep, inverted
		lo_n=rev>>4;		// same here
		printf("Hi =%02x , lo=%02x",hi_n,lo_n);
	
		ttx[0]=0 | ((hi_n)<<4) | 0<<3 | 1<<2;	// e low, Register mode
		ttx[1]=0 | ((hi_n)<<4) | 1<<3 | 1<<2;	// e hig, Register mode
		ttx[2]=0 | ((hi_n)<<4) | 0<<3 | 1<<2;	// e low, Register mode
		// end hinibble,
		// low nibble :
		ttx[3]=0 | ((lo_n)<<4) | 0<<3 | 1<<2;	// e low, Register mode
		ttx[4]=0 | ((lo_n)<<4) | 1<<3 | 1<<2;	// e hig, Register mode
		ttx[5]=0 | ((lo_n)<<4) | 0<<3 | 1<<2;	// e low, Register mode
		// end built
		// prepare delays
		ttx[16+0]=delay;
		ttx[16+1]=delay;
		ttx[16+2]=delay;
		ttx[16+3]=delay;
		ttx[16+4]=delay;
		ttx[16+5]=delay;
		//ttx[16+6]=delay;
		// send
		// prepare addr
		CHAN_addr(_IO24_BOARD,0x160);
		CHAN_setBytes(_IO24_BOARD,32,ttx);	// 32 bytes at a glance , delay compris
		//
		CHAN_addr(_IO24_BOARD,0x154);
		ttx[0]=6;									// we want 8 steps output to be played
		ttx[1]=0;
		CHAN_setBytes(_IO24_BOARD,2,ttx);
		
		
	
		//
		// execute burst
		printf("COMMAND : burst output to D\n");
		CHAN_command(_IO24_BOARD,_BURST_OUTPUT_EXECUTE);
}
//
void burst_print(char* tekst,uint8_t delay)
{
	uint8_t output;
	while(output=*tekst++)
	{
		burst_do_ddram(output,delay);
	}

}
int main(int argc, char* argv[])
{
	char pb[256];
	uint32_t i,j,k;
	uint32_t errors;
	uint32_t argval[16];
	float bat_low,bat_high;
	uint16_t bat_temp;
	uint8_t batval[2];			// holds the 2 bytes
	// chan tx/rx buffer
	uint8_t rxb[256];
	uint8_t txb[256];
	//
	float bat_val;				// actual battery pack voltage, float
	float main_val;				// main voltage float
	float trois_val;			// 3.3v embedded regulator value
	// 0 : BATT LOW REFILL LEVEL
	// 1 : BATT HI REFILL STOP
	if(CHAN_setup("x10DEV",1)!=0) { printf("Error opening Chantilly, quit\n"); exit(-1);}
	errors=CHAN_command(_IO24_BOARD,_STOP);
	//
	errors+=CHAN_addr(_IO24_BOARD,0x154);
	txb[0]=0;	// freezed
	txb[1]=0;	// output index set to zero
	txb[2]=0;	// rollover count : how much
	txb[3]=0;	// n/A
	errors+=CHAN_setBytes(_IO24_BOARD,4,txb);
	printf("errors=%d\n",errors);//errors=0;
	//
	CHAN_command(_IO24_BOARD,_SET_D_OUTPUT);
	//burst_do_command(1,0b00000011,200);	// 4 bit mode ( 1 nibble )
	//burst_do_command(2,0b00101000,200);	// to 4 bit mode
	burst_do_command(1,0b00110000,200);	// -- 0x30
	burst_do_command(1,0b00110000,200);	// -- 0x30
	burst_do_command(1,0b00110000,200);	// -- 0x30
	burst_do_command(1,0b00100000,200);	// -- 0x20
	//burst_do_command(0b00000110,200);	// cursor : auto increment | display shift : OFF
	burst_do_command(2,0b00101000,200);		// 2 lines, 5*8 font
	burst_do_command(2,0b00001111,200);		// display on,cursor on, blink on
	usleep(100000);
	burst_do_command(2,0b00000001,200);		// clear display
	usleep(100000);
	//burst_do_ddram(65,200);
	for(i=0;i<5000;i++)
	{
		burst_do_command(2,0x80,1);				// cursor at line 0,pos 0
		sprintf(pb,"run:%4.4d",i);
		burst_print(pb,1);
		usleep(200000);						//250ms to display
	}
	//
	printf("errors=%d\n",errors);errors=0;
	CHAN_close();
	//
	
	return 0;
}