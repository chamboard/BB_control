#include <stdint.h>
#include <stdio.h>
//
#include <CHANTILLY_BP.h>
//
#define _BB_BOARD 0x07
//
int main(int argc, char* argv[])
{
	uint32_t errors;
	uint8_t poweroff_mode[4];
	uint32_t poweroff_seconds;
	uint16_t poweroff_secs16;
	// chan tx/rx buffer
	uint8_t rxb[256];
	uint8_t txb[256];
	//
	float bat_val;				// actual battery pack voltage, float
	float main_val;				// main voltage float
	float trois_val;			// 3.3v embedded regulator value
	//
	// 0 : BATT LOW REFILL LEVEL
	// 1 : BATT HI REFILL STOP
	//
	if(CHAN_setup("BBHALT",1)!=0) { printf("Error opening Chantilly, quit\n"); CHAN_close();exit(-1);} // try a close
	// get args
	errors=0;
	if(argc==3)	// arg[1]= 0/1 : host only or host+chantilly
	{	
		sscanf(argv[1],"%c",&poweroff_mode);
		printf("\tARG[1]  poweroff_mode  (%c)\n",poweroff_mode[0]);fflush(stdout);
		//
		sscanf(argv[2],"%d",&poweroff_seconds);
		printf("\tARG[2]  poweroff_seconds  (%3.3d)\n",poweroff_seconds);fflush(stdout);
		//
		
		// set poweroff mode,
		if(poweroff_mode[0]=='1') {txb[0]=1;}else{txb[0]=0;}
		txb[1]=0;
		printf("txb[0]=%d  txb[1]=%d\n",txb[0],txb[1]);
		errors+=CHAN_addr(_BB_BOARD,0x120);
		errors+=CHAN_setBytes(_BB_BOARD,2,txb);
		// verify
		errors+=CHAN_addr(_BB_BOARD,0x120);
		errors+=CHAN_getBytes(_BB_BOARD,2,rxb);
		printf("now poweroff value[%02x] \n",rxb[0]);
		//
		// set delay for poweroff, immediate modification
		errors+=CHAN_addr(_BB_BOARD,0x114);
		errors+=CHAN_getBytes(_BB_BOARD,2,rxb);
		printf("LO/HI seconds poweroff original [%02x %02x] \n",rxb[0],rxb[1]);
		//
		poweroff_secs16=(uint16_t) poweroff_seconds;
		errors+=CHAN_addr(_BB_BOARD,0x114);
		errors+=CHAN_setBytes(_BB_BOARD,2,(uint8_t*) &poweroff_secs16);
		
	}
	else
	{
		printf("\n\nMissing parameters!\nUSAGE:\n");
		printf("bbhalt h sss\n");
		printf("h : 0=host only , 1:host+chantilly\n");
		printf("sss : delay in seconds to allow host to execute system's halt command\n");
		printf("------------------------------\n");
		printf("example : sudo ./bbhalt 1 20\n");
		printf("host+chantilly system will be halted, starting 20 seconds after this program's execution\n");
		CHAN_close();
		exit(0);
	}
	//
	//
	// execute poweroff with selected mode or default, 0xBB's ram values
	//
	CHAN_command(_BB_BOARD,0x06);
	printf("Shutting down the system and BusBoard\n");fflush(stdout);
	CHAN_close();
	printf("BBhalt, exit ,errors:%d\n",errors);fflush(stdout);
	system("halt");
	
	return 0;
}