#include <stdint.h>
#include <stdio.h>
//
#include <CHANTILLY_BP.h>
//
typedef float float_t;
#define _BB_BOARD	0x07
#define _IO24_BOARD	0x01
//
#define _EXTVIN_ADJUST  0.5 /* resitor tolerance : user's adjustment if needed */
#define _BB_ADC_ALIASING        (float) 5.0/1024.0
#define _BB_BATTERY_VOLTS_AS_FLOAT(x) (float_t) ((float_t) ((*x | (*(x+1)<<8)) * (_BB_ADC_ALIASING)) / (0.5 ))
#define _BB_BATTERY_FLOAT_AS_UINT16(x) (uint16_t) ( ( x / 2.0 ) / (5.0/1024))
//
#define _BB_MAIN_VOLTS_AS_FLOAT(x) (float_t) ((float_t) ((*x | (*(x+1)<<8)) * (_BB_ADC_ALIASING)) / (4.7/(33.0+4.7+_EXTVIN_ADJUST))) 
#define _BB_HOST_3V3_AS_FLOAT(x)     (float_t) ((float_t) ((*x | (*(x+1)<<8)) * (_BB_ADC_ALIASING)) / ( 1.0 ))
//
int main(int argc, char* argv[])
{
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
	if(CHAN_setup("BB_control",1)!=0) { printf("Error opening Chantilly, quit\n"); exit(-1);}
	// get args
	if(argc==3)	// low battery reload level
	{	
		sscanf(argv[1],"%f",&bat_low);
		printf("\tARG[1]  BATT REFILL LOW  (%5.5f) volt \n",bat_low);
		sscanf(argv[2],"%f",&bat_high);
		printf("\tARG[2]  BATT REFILL HIGH  (%5.5f) volt \n",bat_high);	
		//
		//
		CHAN_addr(_BB_BOARD,0x118);
		CHAN_getBytes(_BB_BOARD,4,rxb);
		printf("LO/HI BATTERY original lo[%02x %02x] hi[%02x %02x]\n",rxb[0],rxb[1],rxb[2],rxb[3]);
		//
		bat_temp=_BB_BATTERY_FLOAT_AS_UINT16(bat_low);
		printf("LO Volts as uint16_t:(%d) ==[%04x]\n",bat_temp,bat_temp);
		// apply bat low,
		CHAN_addr(_BB_BOARD,0x118);
		CHAN_setBytes(_BB_BOARD,2,(uint8_t*) &bat_temp);
		//
		// setup high threshold
		//
		bat_temp=_BB_BATTERY_FLOAT_AS_UINT16(bat_high);
		printf("HI Volts as uint16_t:(%d) ==[%04x]\n",bat_temp,bat_temp);
		// apply lo
		CHAN_addr(_BB_BOARD,0x11a);
		CHAN_setBytes(_BB_BOARD,2,(uint8_t*) &bat_temp);
		//printf("Volts as uint8_t:(%02x)(%02x)\n",(uint8_t*) &bat_temp,(uint8_t*) &bat_temp[1]);
		// update
		CHAN_addr(_BB_BOARD,0x100);
		CHAN_getBytes(_BB_BOARD,6,rxb);		// get main volt, bat volt, host 3v3 values
		bat_val=_BB_BATTERY_VOLTS_AS_FLOAT(&rxb[2]); 
		printf("BAT actual value: %f\n",bat_val);
		main_val=_BB_MAIN_VOLTS_AS_FLOAT(&rxb[0]); 
		printf("MAIN actual value: %f\n",main_val);
		trois_val=_BB_HOST_3V3_AS_FLOAT(&rxb[4]); 
		printf("Onboard 3V3 value: %f\n",trois_val);
	}
	//
	errors=CHAN_addr(_IO24_BOARD,0x100);
	printf("errors=%d\n",errors);
	CHAN_close();
	return 0;
}