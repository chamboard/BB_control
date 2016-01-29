// x10DEVmain.C
//
// dev'ing for IO24 burt output mode on port input B ( mi-cd)
//
// hardware mod : divider resistors,removed, serial resistor 100k replaced with 330ohm
//
// function io24 : 0x0a : set input B as output burst
// function io24 : 0x0c : output data at index, delay with value at index+16
// function io24 : 0x0d : output data at index (32 values max) , with fixed delay value between each data (value located at 0x153 )
//

#include <stdint.h>
#include <stdio.h>
//
#include <CHANTILLY_BP.h>
//
typedef float float_t;
#define _BB_BOARD	0x07
#define _IO24_BOARD	0x01
// x10 command brief
#define _IMMEDIATE					0x01
#define _SERVO_MODE_RUN				0x05
#define _STOP						0x06
#define _SET_D_OUTPUT				0x0a
#define _SET_D_NORMAL				0x0b
#define _BURST_OUTPUT_EXECUTE		0x0c
#define _BURST_FIXOUTPUT_EXECUTE	0x0d
//
// lcd command helpers
//
#define _LCD_CURSOR_0_0		0x80	// ram, line0 pos 0
#define _LCD_CURSOR_1_0		0xC0	// ram , line1 pos 0
//
//
#define _EXTVIN_ADJUST  0.5 /* resitor tolerance : user's adjustment if needed */
#define _BB_ADC_ALIASING        (float) 5.0/1024.0
#define _BB_BATTERY_VOLTS_AS_FLOAT(x) (float_t) ((float_t) ((*x | (*(x+1)<<8)) * (_BB_ADC_ALIASING)) / (0.5 ))
#define _BB_BATTERY_FLOAT_AS_UINT16(x) (uint16_t) ( ( x / 2.0 ) / (5.0/1024))
//
#define _BB_MAIN_VOLTS_AS_FLOAT(x) (float_t) ((float_t) ((*x | (*(x+1)<<8)) * (_BB_ADC_ALIASING)) / (4.7/(33.0+4.7+_EXTVIN_ADJUST))) 
#define _BB_HOST_3V3_AS_FLOAT(x)     (float_t) ((float_t) ((*x | (*(x+1)<<8)) * (_BB_ADC_ALIASING)) / ( 1.0 ))
//
// a lcd command function
//
void burst_do_command(uint8_t nibble_count,uint8_t data,uint8_t fixed_delay)
{
	// special data prepare for lcd 4 bit output bursts
	uint8_t ttx[64];
	uint8_t hi_n,lo_n;
	uint8_t rev;
	//
	rev=data;	// now normal
	//
	// end reverse
	hi_n=rev>>4;	// yep, inverted
	lo_n=rev&0b1111;		// same here
#ifdef _DEBUG_THIS
	printf("command burst (Hi =%01x , lo=%01x",hi_n,lo_n);
#endif
	
	ttx[0]=0 | ((hi_n)<<4) | 0<<3 | 0<<2;	// e low, Register mode
	ttx[1]=0 | ((hi_n)<<4) | 1<<3 | 0<<2;	// e hig, Register mode
	ttx[2]=0 | ((hi_n)<<4) | 0<<3 | 0<<2;	// e low, Register mode
	// end hinibble,
	// low nibble :
	ttx[3]=0 | ((lo_n)<<4) | 0<<3 | 0<<2;	// e low, Register mode
	ttx[4]=0 | ((lo_n)<<4) | 1<<3 | 0<<2;	// e hig, Register mode
	ttx[5]=0 | ((lo_n)<<4) | 0<<3 | 0<<2;	// e low, Register mode
	// end built
	// send
	// prepare addr
	CHAN_addr(_IO24_BOARD,0x160);
	CHAN_setBytes(_IO24_BOARD,6,ttx);	// 6 bytes at a glance in case of hi+lo nibble (could be optimized)
	// sets fixed delay time , data count , data start index at once
	CHAN_addr(_IO24_BOARD,0x153);
	ttx[0]=fixed_delay;
	ttx[1]=nibble_count*3;//6;									// we want 8 steps output to be played
	ttx[2]=0;
	ttx[3]=0;
	CHAN_setBytes(_IO24_BOARD,4,ttx);	
	//
	// execute burst
	#ifdef _DEBUG_THIS
	printf("COMMAND : burst fixe delay output to D\n");
#endif
	CHAN_command(_IO24_BOARD,_BURST_FIXOUTPUT_EXECUTE);
}
//
// a buffering sub function
//
uint8_t burst_prepare_ddram(uint8_t nibble_count,uint8_t data,uint8_t index)
{
	// special data prepare for lcd 4 bit output bursts
	uint8_t ttx[64];
	uint8_t hi_n,lo_n;
	uint8_t rev;
	rev=data;
	// end reverse
	// split nibbles
	hi_n=rev>>4;	// yep, inverted
	lo_n=rev&0b1111;		// same here
	#ifdef _DEBUG_THIS
	printf("Hi =%02x , lo=%02x",hi_n,lo_n);
#endif
	
	ttx[0]=0 | ((hi_n)<<4) | 0<<3 | 1<<2;	// e low, Register mode
	ttx[1]=0 | ((hi_n)<<4) | 1<<3 | 1<<2;	// e hig, Register mode
	ttx[2]=0 | ((hi_n)<<4) | 0<<3 | 1<<2;	// e low, Register mode
	// end hinibble,
	// low nibble :
	ttx[3]=0 | ((lo_n)<<4) | 0<<3 | 1<<2;	// e low, Register mode
	ttx[4]=0 | ((lo_n)<<4) | 1<<3 | 1<<2;	// e hig, Register mode
	ttx[5]=0 | ((lo_n)<<4) | 0<<3 | 1<<2;	// e low, Register mode
	// end built
	// prepare addr
	CHAN_addr(_IO24_BOARD,0x160+index);
	CHAN_setBytes(_IO24_BOARD,nibble_count*3,ttx);	// 6 bytes at a glance , delay compris
	//
	return (index+(nibble_count*3));											// returns new index
}
//
// splits intelligent string
//
void burst_out(char*tekst,uint8_t common_delay)
{
	uint8_t ttx[32];
	uint8_t output;
	uint8_t max_op=30;// 10*3 data = 30 , 32 is the max
	// lcd : each op=2 bytes (suppose E is low at start)
	uint8_t cursor,newcursor;
	cursor=0;
	while(output=*tekst++)
	{
		#ifdef _DEBUG_THIS
		printf("cur=%d",cursor);
#endif
		newcursor=burst_prepare_ddram(2,output,cursor);// assume we are in 4bit mode, 2 nibbles to be sent
		if(newcursor>=30)
		{
			// execute that chunk
			CHAN_addr(_IO24_BOARD,0x154);
			ttx[0]=newcursor;//6;									// we want 8 steps output to be played
			ttx[1]=0;
			ttx[2]=0;
			ttx[3]=0;
			CHAN_setBytes(_IO24_BOARD,4,ttx);	
			//
			// execute burst
			#ifdef _DEBUG_THIS
			printf("COMMAND : burst fix delay output to D\n");
#endif
			CHAN_command(_IO24_BOARD,_BURST_FIXOUTPUT_EXECUTE);
			// resets cursor for next chunk
			newcursor=0;
		}
		cursor=newcursor;
	}
	// end buffering, execute
	// execute that chunk
	CHAN_addr(_IO24_BOARD,0x154);
	ttx[0]=newcursor;//6;									// we want 8 steps output to be played
	ttx[1]=0;
	ttx[2]=0;
	ttx[3]=0;
	CHAN_setBytes(_IO24_BOARD,4,ttx);	
	//
	// execute burst
	#ifdef _DEBUG_THIS
	printf("COMMAND : burst fix delay output to D\n");
#endif
	CHAN_command(_IO24_BOARD,_BURST_FIXOUTPUT_EXECUTE);
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
	if(CHAN_setup("x10DEV",1)!=0) { printf("Error opening Chantilly, quit\n"); CHAN_close();exit(-1);}//exit trying a close before
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
	//
	burst_do_command(1,0b00110000,200);	// -- 0x30
	burst_do_command(1,0b00110000,200);	// -- 0x30
	burst_do_command(1,0b00110000,200);	// -- 0x30
	burst_do_command(1,0b00100000,200);	// -- 0x20 set interface : 4 bit mode  partir de la.
	//
	burst_do_command(2,0b00101000,200);		// 2 lines, 5*7 font
	burst_do_command(2,0b00001111,200);		// display on,cursor on, blink on
	usleep(100000);
	burst_do_command(2,0b00000001,200);		// clear display
	usleep(100000);
	// test: disable cursor
	burst_do_command(2,0b00001100,200);		// Display=ON Cursor=OFF Blink=OFF
	for(i=0;i<50;i++)
	{
		burst_do_command(2,_LCD_CURSOR_0_0,1);				// cursor at line 0,pos 0
		sprintf(pb,"run:%4.4d",i);
		burst_out(pb,1);
		usleep(20000);						//250ms to display
	}
	burst_do_command(2,_LCD_CURSOR_1_0,1);				// cursor at line 1,pos 0
	burst_out("0123456789abcdef",10);
	//
	usleep(1000000);
	// read adc vals, display
	for(i=0;i<100;i++)
	{
		CHAN_addr(_BB_BOARD,0x100);
		CHAN_getBytes(_BB_BOARD,6,rxb);		// get main volt, bat volt, host 3v3 values
		//
		// top line, main
		main_val=_BB_MAIN_VOLTS_AS_FLOAT(&rxb[0]); 
		sprintf(pb,"MAIN:%3.1fV ",main_val);
		burst_do_command(2,_LCD_CURSOR_0_0,1);				// cursor at line 0,pos 0
		burst_out(pb,1);
		//
		bat_val=_BB_BATTERY_VOLTS_AS_FLOAT(&rxb[2]); 
		trois_val=_BB_HOST_3V3_AS_FLOAT(&rxb[4]); 
		sprintf(pb,"3V3:%3.1fV BAT:%3.1fV",trois_val,bat_val);
		burst_do_command(2,_LCD_CURSOR_1_0,1);				// cursor at line 1,pos 0
		burst_out(pb,1);
		usleep(250000);	// delay between bat scans
	}
		
	printf("errors=%d\n",errors);errors=0;
	CHAN_close();
	//
	
	return 0;
}