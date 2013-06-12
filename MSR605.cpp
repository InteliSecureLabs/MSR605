/*
 *  MSR605.cpp
 * 
 *  MSR605 Application, for operating a MSR605 magstripe reader/writer.
 *  Copyright (C) 2013 Pentura
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include "msr605.h"

#define DEVICE "/dev/ttyUSB0"

MSR605 *msr = new MSR605();

void sigproc(int x)
{ 		
		printf("Quitting...");
		msr->disconnect();
		delete(msr);
		exit(0);
}

void printTrack(const char *msg, unsigned char *buf, unsigned int len)
{
	printf("size: %d",len);
	printf("[%s]: ", msg);
	for(int x = 0; x < len; x++) printf("%02x", buf[x]);
	printf("\n");
}

void printTrackiso(const char *msg, unsigned char *buf, unsigned int len)
{
	printf("size: %d",len);
	printf("[%s]: ", msg);
	for(int x = 0; x < len; x++) printf("%c", buf[x]);
	printf("\n");
}

void license(){
	printf("MSR605  Copyright (C) 2013 Pentura \nThis program comes with ABSOLUTELY NO WARRANTY; \nThis is free software, and you are welcome to redistribute it under certain conditions;\n\n");
}

int main (int argc, const char * const argv[]) {
  
	if ( argc != 5 ){
	      license();
	      printf( "\tusage: %s <track1 bit> <track2 bit> <track3 bit> <mode>\n", argv[0] );
	}else{
	  int t1 = atoi(argv[1]);
	  int t2 = atoi(argv[2]);
	  int t3 = atoi(argv[3]);
	  int mode = atoi(argv[4]);
	  if (t1 < 5|| t1 > 8 || t1 == 6) {
	    printf("ERROR: Track 1 bits must be either 5,7 or 8\n");
	  
	  }
	  if (t2 < 5 || t2 > 8|| t2== 6) {
	    printf("ERROR: Track 2 bits must be either 5,7 or 8\n");
	  
	  }
	  if (t3 < 5 || t3 > 8|| t3 == 6) {
	    printf("ERROR: Track 3 bits must be either 5,7 or 8\n"); 
	  }
  
	license();
	signal(SIGINT, sigproc);	
	try 
	{
		/* connect to specified device */
		magnetic_stripe_t *data = NULL;
		msr->connect(DEVICE);
		printf("Connected to %s\n", DEVICE);
		msr->sendReset();
		msr->getFirmware();
		msr->getModel();
		msr->sendReset();
                
		/* initialize the msr */
		msr->init();
		printf("Initialized MSR605.\n");

		/* read card */
		while(1) {
		  
		  msr->setAllLEDOff();
		  printf("Waiting for swipe...\n");
		  switch(mode){
		    case 1:
			    data=msr->readCard_raw(t1, t2, t3);
			    break;
		    case 2:
			    data=msr->readCard_iso(t1, t2, t3);
			    break;
		  }
		  //printTrack("Track 1", data->track1, data->t1_len);
		  //printTrack("Track 2", data->track2, data->t2_len);
		  //printTrack("Track 3", data->track3, data->t3_len);
		  
		  printTrackiso("Track 1", data->track1, data->t1_len);
		  printTrackiso("Track 2", data->track2, data->t2_len);
		  printTrackiso("Track 3", data->track3, data->t3_len);
		
		  //msr->free_ms_data(data);
		}
				
		/* close connection */
		msr->disconnect();

	}
	catch(const char *msg)
	{
		printf("MSR605 Error: %s\n", msg);
	}
	
	
	delete(msr);
    
	return 0;

	  
	}
}
