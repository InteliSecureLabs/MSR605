/*
 *  MSR605.h
 *  msr605-lib
 *
 *  Created by Chris Moos on 8/5/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSR605_H
#define MSR605_H

#define BAUDRATE 9600

/* MSR 206 Commands */
#define MSR_RESET "\x1b\x61"
#define MSR_COMM_TEST "\x1b\x65"
#define MSR_COMM_TEST_ACK "\x1b\x79"
#define MSR_RED_LED_ON "\x1b\x85"
#define MSR_GREEN_LED_ON "\x1b\x83"
#define MSR_YELLOW_LED_ON "\x1b\x84"
#define MSR_ALL_LIGHTS_OFF "\x1b\x81"
#define MSR_ALL_LIGHTS_ON "\x1b\x82"
#define MSR_SET_BPC "\x1b\x6f"
#define MSR_SET_BPC_ACK "\x1b\x30"
#define MSR_READ_RAW "\x1b\x6d"
#define MSR_READ_ACK "\x1b\x73"
#define MSR_WRITE_ISO "\x1b\x77"
#define MSR_CHECK_ZEROS "\x1b\x6c"
#define MSR_END_READ "\x3f\x1c\x1b"
#define MSR_END_READ "\x3f\x1c\x1b"
#define MSR_HICO "\x1b\x78"
#define MSR_LOCO "\x1b\x79"

#define MSR_WRITE_RAW "\x1b\x6e"
#define MSR_READ_ISO "\x1b\x72"


/* MSR Status Byte Read */
#define MSR_STATUS_OK 0x30
#define MSR_STATUS_FAIL 0x41
#define MSR_STATUS_WRITE_READ_ERROR 0x31
#define MSR_STATUS_COMMAND_FORMAT_ERROR 0x32
#define MSR_STATUS_INVALID_COMMAND 0x34
#define MSR_STATUS_INVALID_SWIPE_WRITE_MODE 0x39

#define MSR_TRACK_1 "\x1b\x01"
#define MSR_TRACK_2 "\x1b\x02"
#define MSR_TRACK_3 "\x1b\x03"

/* MSR 206 BPC */

/* Track Options */
#define TRACK_7BIT 7
#define TRACK_8BIT 8
#define TRACK_5BIT 5


using namespace std;

typedef struct magnetic_stripe_t
{
	unsigned char *track1, *track2, *track3;
	unsigned int t1_len, t2_len, t3_len;
};

typedef struct leading_zeros_t 
{
	char t1t3;
	char t2;
};

class MSR605
{
	public:
		MSR605();
		~MSR605();
		
		void connect(char *devName); /* throws an exception if unable to connect */
		bool isConnected();
		void disconnect();
		
		/* read/write */
		int read_bytes(unsigned char *buf, int num);
		int write_bytes(char *buf, int num);
		
		/* card commands */
		magnetic_stripe_t *readCard_raw(char track1_format, char track2_format, char track3_format);
                magnetic_stripe_t *readCard_iso(char track1_format, char track2_format, char track3_format);

		
		/* utility functions */
		void print_bytes(unsigned char *bytes, int len);
		void decode_5bit(unsigned char *buf, unsigned int len, unsigned char * &outBuf, unsigned int &outLen);
		void decode_7bit(unsigned char *buf, unsigned int len, unsigned char * &outBuf, unsigned int &outLen);
		void decode_8bit(unsigned char *buf, unsigned int len, unsigned char * &outBuf, unsigned int &outlen);
		void readTrack1(unsigned char * &outBuf, unsigned int &outLen, char trackOptions);
		void readTrack23(unsigned char * &outBuf, unsigned int &outLen, char trackOptions);
                void readTrack_raw(unsigned char * &outBuf, unsigned int &outLen, char trackOptions);
		

		/* memory stuff */
		void free_ms_data(magnetic_stripe_t *ms_data);
		
		/* commands */
		void init();
		void sendReset();
		bool commTest(); /* alerts us if we are communicating properly with device */
		void setRedLEDOn();
		void setGreenLEDOn();
		void setYellowLEDOn();
		void setAllLEDOff();
		void setAllLEDOn();
		bool setBPC(char track1, char track2, char track3); /* set bit per character for each track */
		void getLeadingZeros(leading_zeros_t *zeros);
		void getFirmware();
		void getModel();
		
	private:
		int fd;
};

#endif


