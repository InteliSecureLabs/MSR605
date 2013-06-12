/*
 *  MSR605.cpp
 *  msr605-lib
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
 *
 *
 */

#include "msr605.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <iostream>
#include <signal.h>

//#define DEBUG

using namespace std;

//VERSION ALPHA

static inline uint8_t swap_bits(uint8_t n) {
  n = ((n&0xF0) >>4 ) | ( (n&0x0F) <<4);
  n = ((n&0xCC) >>2 ) | ( (n&0x33) <<2);
  n = ((n&0xAA) >>1 ) | ( (n&0x55) <<1);

  return  n;
};

void catch_alarm(int sig_num)
{
    printf("MSR605 Timed out! Exiting...\n\n");
    exit(0);
}


MSR605::MSR605()
{
	fd = -1;
}
/*-------------------------------------------------------------------------------------*/
MSR605::~MSR605()
{

}
/*-------------------------------------------------------------------------------------*/
bool MSR605::isConnected()
{
	if(this->fd > 0) return true;
	return false;
}
/*-------------------------------------------------------------------------------------*/
void MSR605::init()
{
	if(!isConnected()) throw "Unable to initialize: Not connected to device.";
	this->setRedLEDOn();
	sleep(1);
	this->setYellowLEDOn();
	sleep(1);
	this->setGreenLEDOn();
	sleep(1);
	this->setAllLEDOff();
	if(!this->commTest()) throw "Unable to initialize: Communications Test failed.";
	this->sendReset();
}
/*-------------------------------------------------------------------------------------*/
int MSR605::write_bytes(char *buf, int num)
{
	int ret = 0;
	
	if(!isConnected()) throw "Unable to send data: Not connected to device.";
	ret = write(this->fd, buf, num);
	if(ret <= 0) throw "Unable to send data: write() failed";

	#ifdef DEBUG
	printf("Sent Data(%d bytes): ", num);
	for(int x = 0; x < num; x++) printf("%02x ", buf[x]);
	printf("\n");
	#endif
	
	return ret;
}
/*-------------------------------------------------------------------------------------*/
int MSR605::read_bytes(unsigned char *buf, int num)
{
	int temp = 0;
	int ret;
	
	if(buf == NULL) return -1;
	
	
	while(temp != num) {
		ret = read(this->fd, buf+temp, num - temp);
		if(ret < 0) return -1;
		if(ret > 0) temp += ret;
	}
	
	#ifdef DEBUG
	printf("Received Data(%d bytes): ", num);
	for(int x = 0; x < num; x++) printf("%02x ", buf[x]);
	printf("\n");
	#endif
	
	return num;
}
/*-------------------------------------------------------------------------------------*/
void MSR605::readTrack1(unsigned char * &outBuf, unsigned int &outLen, char trackOptions)
{
	unsigned char buf[256];
	unsigned char buf2[256];
	unsigned int len=0;
	/* track header */
	read_bytes((unsigned char*)&buf, 1);
	
	if(memcmp(buf, "\x1b", 1) != 1) {
	    read_bytes((unsigned char*)&buf, 1);
	    printf("Track Header %02x\n",buf[0]);
	
	    for(int i=0;i<254;i++){
	      read_bytes((unsigned char*)&buf, 1);
	      if(memcmp(buf,"\x1b", 1) == 0) {
		    break;
	      }else{
		    buf2[i]=buf[0];
		    len=i;
		//do nothing
	      }
	    }
	}

	len++;
	#ifdef DEBUG
	printf("\nTrying to read data: %d\n",len);
	#endif
	//read_bytes((unsigned char*)&buf2, len);
	decode_8bit((unsigned char*)&buf2, len, outBuf, outLen);	
	
	//if(trackOptions == TRACK_7BIT) decode_7bit((unsigned char*)&buf2, len, outBuf, outLen);
	//if(trackOptions == TRACK_5BIT) decode_5bit((unsigned char*)&buf2, len, outBuf, outLen);
	//if(trackOptions == TRACK_8BIT) decode_8bit((unsigned char*)&buf2, len, outBuf, outLen);
	#ifdef DEBUG
	printf("\nTrack complete\n");
	#endif
}

void MSR605::readTrack23(unsigned char * &outBuf, unsigned int &outLen, char trackOptions)
{
	unsigned char buf[256];
	unsigned char buf2[256];
	unsigned int len=0;
	/* track header */
	read_bytes((unsigned char*)&buf, 1);
	
	 printf("Track Header %02x\n",buf[0]);
	
	    for(int i=0;i<254;i++){
	      read_bytes((unsigned char*)&buf, 1);
	      if(memcmp(buf,"\x1b", 1) == 0) {
		    break;
	      }else{
		    buf2[i]=buf[0];
		    len=i;
		//do nothing
	      }
	    }
	len++;
	#ifdef DEBUG
	printf("\nTrying to read data: %d\n",len);
	#endif
	//read_bytes((unsigned char*)&buf2, len);
	decode_8bit((unsigned char*)&buf2, len, outBuf, outLen);	
	
	//if(trackOptions == TRACK_7BIT) decode_7bit((unsigned char*)&buf2, len, outBuf, outLen);
	//if(trackOptions == TRACK_5BIT) decode_5bit((unsigned char*)&buf2, len, outBuf, outLen);
	//if(trackOptions == TRACK_8BIT) decode_8bit((unsigned char*)&buf2, len, outBuf, outLen);
	#ifdef DEBUG
	printf("\nTrack complete\n");
	#endif
}

void MSR605::readTrack_raw(unsigned char * &outBuf, unsigned int &outLen, char trackOptions)
{
	unsigned char buf[512];
	unsigned int len;
	
	/* track header */
	read_bytes((unsigned char*)&buf, 2);
	if(memcmp(buf, "\x1b", 1) != 0) {
		throw "Unable to read data: Expected Track Header";
	}

	printf("Track Header %02x %02x\n",buf[0],buf[1]);

	/* track length */
	if(read_bytes((unsigned char*)&buf, 1) != 1) {
		throw "Unable to read data: Expected Track Length";
	}
	
	printf("Track Length  %02x \n",buf[0]);

	if(buf[0] > 254) {
		throw "Unable to read data: Invalid length received";
	}
	else if(buf[0] == 0) { /* no data for this track */
		outBuf = NULL;
		outLen = 0;
		return;
	}
	
	len = buf[0];
	
	
	/* track data */
	read_bytes((unsigned char*)&buf, len);
	
	if(trackOptions == TRACK_7BIT) decode_7bit((unsigned char*)&buf, len, outBuf, outLen);
	if(trackOptions == TRACK_5BIT) decode_5bit((unsigned char*)&buf, len, outBuf, outLen);
	if(trackOptions == TRACK_8BIT) decode_8bit((unsigned char*)&buf, len, outBuf, outLen);
	
}
/*-------------------------------------------------------------------------------------*/
void MSR605::free_ms_data(magnetic_stripe_t *ms_data)
{
	if(ms_data == NULL) return;
	
	if(ms_data->track1 != NULL) free(ms_data->track1);
	if(ms_data->track2 != NULL) free(ms_data->track2);
	if(ms_data->track3 != NULL) free(ms_data->track3);
	
	free(ms_data);
}
/*-------------------------------------------------------------------------------------*/
magnetic_stripe_t *MSR605::readCard_raw(char track1_format, char track2_format, char track3_format)
{
	unsigned char buf[512];
	unsigned char end[3];
	magnetic_stripe_t *ms_data = (magnetic_stripe_t*)malloc(sizeof(magnetic_stripe_t));
	
	if(!isConnected()) {
		free_ms_data(ms_data);
		throw "Unable to read card: Not connected to device.";
	}
	
	/* set track format */
	this->setBPC(track1_format, track2_format, track3_format);
	
	
	write_bytes(MSR_READ_RAW, 2);
	
	/* check for ack */
	read_bytes((unsigned char*)&buf, 2);
	if(memcmp(buf, MSR_READ_ACK, 2) != 0) {
		free_ms_data(ms_data);
		throw "Unable to read data: Invalid Response";
	}

	readTrack_raw(ms_data->track1, ms_data->t1_len, track1_format);
	readTrack_raw(ms_data->track2, ms_data->t2_len, track2_format);
	readTrack_raw(ms_data->track3, ms_data->t3_len, track3_format);
	
	/* read end */
	read_bytes((unsigned char*)&end, 3);
	if(memcmp(end, MSR_END_READ, 3) != 0) {
		free_ms_data(ms_data);
		throw "Unable to read data: Invalid end response";
	}
	
	/* read status byte */
	read_bytes((unsigned char*)&end, 1);
	if(end[0] == MSR_STATUS_OK) {
		return ms_data;
	}
	else { /* error messages */
		free_ms_data(ms_data);
		switch(end[0])
		{
			case MSR_STATUS_WRITE_READ_ERROR:
				throw "Unable to read card: Write or read error";
				
			case MSR_STATUS_COMMAND_FORMAT_ERROR:
				throw "Unable to read card: Command format error";
				
			case MSR_STATUS_INVALID_COMMAND:
				throw "Unable to read card: Invalid command";
				
			case MSR_STATUS_INVALID_SWIPE_WRITE_MODE:
				throw "Unable to read card: Invalid card swipe when in write mode";
				
			default:
				throw "Unable to read card: Unknown error code";
		}
	}
	
	
	return ms_data;
}

magnetic_stripe_t *MSR605::readCard_iso(char track1_format, char track2_format, char track3_format)
{
	unsigned char buf[1024];
	unsigned char end[3];
	magnetic_stripe_t *ms_data = (magnetic_stripe_t*)malloc(sizeof(magnetic_stripe_t));
	
	if(!isConnected()) {
		free_ms_data(ms_data);
		throw "Unable to read card: Not connected to device.";
	}
	
	/* set track format */
	this->setBPC(track1_format, track2_format, track3_format);
	
	
	write_bytes(MSR_READ_ISO, 2);
	
	/* check for ack */
	read_bytes((unsigned char*)&buf, 2);
	if(memcmp(buf, MSR_READ_ACK, 2) != 0) {
		free_ms_data(ms_data);
		throw "Unable to read data: Invalid Response";
	}

	readTrack1(ms_data->track1, ms_data->t1_len, track1_format);
	readTrack23(ms_data->track2, ms_data->t2_len, track2_format);
	readTrack23(ms_data->track3, ms_data->t3_len, track3_format);
	
	/* read end */
	read_bytes((unsigned char*)&end, 1);
	read_bytes((unsigned char*)&end, 3);
	if(memcmp(end, MSR_END_READ, 3) != 0) {
		free_ms_data(ms_data);
		throw "Unable to read data: Invalid end response";
	}
	
	/* read status byte */
	read_bytes((unsigned char*)&end, 1);
	if(end[0] == MSR_STATUS_OK) {
		printf("READ OK!\n");
		//return ms_data;
	}
	else { /* error messages */
		free_ms_data(ms_data);
		switch(end[0])
		{
			case MSR_STATUS_WRITE_READ_ERROR:
				throw "Unable to read card: Write or read error";
				
			case MSR_STATUS_COMMAND_FORMAT_ERROR:
				throw "Unable to read card: Command format error";
				
			case MSR_STATUS_INVALID_COMMAND:
				throw "Unable to read card: Invalid command";
				
			case MSR_STATUS_INVALID_SWIPE_WRITE_MODE:
				throw "Unable to read card: Invalid card swipe when in write mode";
				
			default:
				throw "Unable to read card: Unknown error code";
		}
	}
	
	
	return ms_data;
}
/*-------------------------------------------------------------------------------------*/
void MSR605::getLeadingZeros(leading_zeros_t *zeros)
{
	unsigned char buf[3];
	
	if(!isConnected()) throw "Unable to check leading zeros: Not connected to device.";
	if(zeros == NULL) throw "Invalid leading zeros structure passed";
	
	write_bytes(MSR_CHECK_ZEROS, 2);
	
	if(read_bytes((unsigned char*)&buf, 3) != 3) throw "Unable to check leading zeros: invalid response";
	if(memcmp(buf, "\x1b", 1) != 0) throw "Unable to check leading zeros: bad resposne";
	
	zeros->t1t3 = buf[1];
	zeros->t2 = buf[2];
}
/*-------------------------------------------------------------------------------------*/
void MSR605::decode_7bit(unsigned char *buf, unsigned int len, unsigned char * &outBuf, unsigned int &outLen)
{
	unsigned int bytes = (len * 8) / 7;
	outBuf = (unsigned char*)malloc(bytes);
	unsigned int tempLen = 0;
	char *test;
	
	test = (char*)outBuf;

	for(int y = 0; y < (len/7); y++) {
	
	if(*buf == 0x00 || buf[1] == 0x00) break;
	
	test[0] = swap_bits(buf[0] & 0xfc) + 0x20;
	test[1] = swap_bits(((buf[1] & 0xf8) >> 1) | ((buf[0] & 1) << 7)) + 0x20;
	test[2] = swap_bits(((buf[2] & 0xf0) >> 2) | ((buf[1] & 3) << 6)) + 0x20; 
	test[3] = swap_bits(((buf[3] & 0xe0) >> 3) | ((buf[2] & 7) << 5)) + 0x20;
	test[4] = swap_bits(((buf[4] & 0xc0) >> 4) | ((buf[3] & 0xf) << 4)) + 0x20;
	test[5] = swap_bits(((buf[5] & 0x80) >> 5) | ((buf[4] & 0x1f) << 3)) + 0x20;
	test[6] = swap_bits((buf[5] & 0x3f) << 2) + 0x20;
	test[7] = swap_bits((buf[6] & 0x7e) << 1) + 0x20;
	
	tempLen += 8;
	buf += 7;
	test += 8;
	
	}
	
	outLen = tempLen;
}
/*-------------------------------------------------------------------------------------*/
void MSR605::decode_5bit(unsigned char *buf, unsigned int len, unsigned char * &outBuf, unsigned int &outLen)
{
	unsigned int bytes = (len * 8) / 5;
	outBuf = (unsigned char*)malloc(bytes);
	unsigned int tempLen = 0;
	char *test;
	
	test = (char*)outBuf;
	
	for(int y = 0; y < (len/5); y++) {
	
	if(*buf == 0x00 || buf[1] == 0x00) break;
	
	test[0] = swap_bits(buf[0] & 0xf0) + 0x30; //f0  was f8 corrected due to parity errors when buf[0]=1!
	test[1] = swap_bits(((buf[1] & 0x80) >> 3) | ((buf[0] & 7) << 5)) + 0x30;
	test[2] = swap_bits((buf[1] & 0x3c) << 2) + 0x30; 
	test[3] = swap_bits(((buf[2] & 0xe0) >> 1) | ((buf[1] & 1) << 7)) + 0x30;
	test[4] = swap_bits((buf[2] & 0xf) << 4) + 0x30;
	test[5] = swap_bits((buf[3] & 0x78) << 1) + 0x30;
	test[6] = swap_bits((buf[4] & 0xc0) >> 2 | ((buf[3] & 3) << 6)) + 0x30;
	test[7] = swap_bits((buf[4] & 0x1e) << 3) + 0x30;
	
	tempLen += 8;
	buf += 5;
	test += 8;
	
	}
	
	outLen = tempLen;
}

void MSR605::decode_8bit(unsigned char *buf, unsigned int len, unsigned char * &outBuf, unsigned int &outLen)
{
	//unsigned int bytes = len;
	unsigned int bytes = (len * 8) /8;
	outBuf = (unsigned char *)malloc(len);
	unsigned int tempLen = 0;
	//char *test;
	//unsigned int *len_ptr=len;
	char *test;	
	test=(char*)outBuf;
	for(int y = 0; y < (len/8); y++) {
	if(*buf == 0x00 || buf[1] == 0x00) break;
	
	test[0] =buf[0]; 
	test[1] =buf[1]; 
	test[2] = buf[2];  
	test[3] = buf[3]; 
	test[4] = buf[4]; 
	test[5] = buf[5]; 
	test[6] = buf[6]; 
	test[7] = buf[7];
	
	tempLen += 8;
	buf += 8;
	test += 8;
	
	}
	outLen=len;

}
/*-------------------------------------------------------------------------------------*/
void MSR605::disconnect()
{
	if(!isConnected()) throw "Unable to close connection: not connected";
	sendReset();
	close(this->fd);
}
/*-------------------------------------------------------------------------------------*/
void MSR605::print_bytes(unsigned char *bytes, int len)
{
	printf("Bytes: ");
	for(int x = 0; x < len; x++) printf("%02x ", bytes[x]);
	printf("\n");
}
/*-------------------------------------------------------------------------------------*/
bool MSR605::setBPC(char track1, char track2, char track3)
{
	char ack[5];
	char bpc_str[6];
	
	if(!isConnected()) throw "Unable to set bpc: not connected";
	
	memcpy(bpc_str, (void*)MSR_SET_BPC, 2);
	memcpy(&bpc_str[2], (void*)&track1, 1);
	memcpy(&bpc_str[3], (void*)&track2, 1);
	memcpy(&bpc_str[4], (void*)&track3, 1);
	
	
	write_bytes(bpc_str, 5);
	
	if(read_bytes((unsigned char*)&ack, 5) != 5) throw "Unable to set BPC: invalid response";
	if(memcmp(ack, MSR_SET_BPC_ACK, 5) == 0) return true;
	
	return false;
}
/*-------------------------------------------------------------------------------------*/
bool MSR605::commTest()
{
	char ack[2];
	
	signal(SIGALRM, catch_alarm);	

	if(!isConnected()) throw "Unable to perform comm test: not connected";
	
	write_bytes(MSR_COMM_TEST, 2);
	printf("Comm Test Sent...\n");	
	//alarm(6);
	if(read_bytes((unsigned char*)&ack, 2) != 2) throw "Comm test failed: invalid response";
	//alarm(6);
	printf("Receiving Response.\n");
	if(memcmp(ack, MSR_COMM_TEST_ACK, 2) == 0) return true;
	
	return false;
}
/*-------------------------------------------------------------------------------------*/
void MSR605::setRedLEDOn()
{
	if(!isConnected()) throw "Unable to turn Red LED on: not connected";
	write_bytes(MSR_RED_LED_ON, 2);
}
void MSR605::setGreenLEDOn()
{
	if(!isConnected()) throw "Unable to turn Green LED on: not connected";
	write_bytes(MSR_GREEN_LED_ON, 2);
}
void MSR605::setYellowLEDOn()
{
	if(!isConnected()) throw "Unable to turn Yellow LED on: not connected";
	write_bytes(MSR_YELLOW_LED_ON, 2);
}
/*-------------------------------------------------------------------------------------*/
void MSR605::setAllLEDOn()
{
	if(!isConnected()) throw "Unable to turn all LEDs on: not connected";
	write_bytes(MSR_ALL_LIGHTS_ON, 2);
}
/*-------------------------------------------------------------------------------------*/
void MSR605::setAllLEDOff()
{
	if(!isConnected()) throw "Unable to turn all LEDs off: not connected";
	write_bytes(MSR_ALL_LIGHTS_OFF, 2);
}
/*-------------------------------------------------------------------------------------*/
void MSR605::sendReset()
{
	if(!isConnected()) throw "Unable to send reset: not connected";
	
	write_bytes(MSR_RESET, 2);
}
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
void MSR605::connect(char *devName)
{
	struct termios options;
	
	if(devName == NULL) throw "Invalid device name specified.";
	
	/* open connection to device */
	this->fd = open(devName, O_RDWR | O_NOCTTY);
	if(this->fd < 0) throw "Unable to open connection to device.";
	
	/* get options for terminal session */
	tcgetattr(this->fd, &options);
	
	/* set options */
	options.c_cflag = CS8 | CREAD | CLOCAL;
	options.c_oflag = 0;
	options.c_iflag = 0;
	
	/* set baud rate */
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	
	/* push options */
	tcsetattr(this->fd, TCSANOW, &options);
	
}
/*-------------------------------------------------------------------------------------*/
void MSR605::getModel()
{
	char *model = (char *)malloc(sizeof(char *) * 3);
	write_bytes("\x1b\x74", 2);
	read_bytes((unsigned char*)model, 3);
	model[2]='\0';
	memmove (model, model+1, strlen (model));
	printf("Model: %s\n",model);
	free(model);
}
/*-------------------------------------------------------------------------------------*/

void MSR605::getFirmware()
{ 
	char *firmware= (char *)malloc(sizeof(char *) * 9);
	write_bytes("\x1b\x76", 2);
	read_bytes((unsigned char*)firmware, 9);
	firmware[9]='\0';
	memmove (firmware, firmware+1, strlen (firmware));
	printf("Firmware: %s\n",firmware);
	free(firmware);
}

