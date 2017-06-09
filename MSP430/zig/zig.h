#ifndef ZIG_H
#define ZIG_H
#include <stdio.h>
#include <stdlib.h>
#include "msp430g2553.h"
#include "spi.h"
#include "zigaddr.h"
#include "general2.h"

#define FAIL -1
#define SUCCESS 0

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef unsigned char* BYTEPNT;

void zig_RstSoft ();

void zig_RstRF ();

BYTE zig_GetShort (BYTE address);
	
void zig_SetShort (BYTE address, BYTE value);

BYTE zig_GetLong (WORD address);

void zig_SetLong (WORD address, BYTE value);

void zig_SelChannel (BYTE channel);

WORD zig_ContiguousWrite (WORD addr, BYTEPNT mem, int count);

WORD zig_ContiguousRead (WORD addr, BYTEPNT mem, int count);

int zig_TX_Transmit ();

int zig_RX_Receive ();

void zig_CheckInterrupt ();

void zig_Init (BYTE channel, BYTE srcAddrLong[], BYTE srcAddrShort[], BYTE srcPANid[]);

void zig_TX_config (BYTE frameType, BYTE ackRequired, BYTE PANcomp, 
				BYTE noSequenceNum, BYTE dstAddrMode, BYTE srcAddrMode);

void zig_configRadioAddr (BYTE srcAddrLong[], BYTE srcAddrShort[], BYTE srcPANid[]);

void zig_TX_configDstAddr (BYTE dstAddr[], BYTE dstPANid[]);

void zig_TX_PayloadToBuffer (BYTEPNT payload, BYTE payloadSize);

typedef struct							
{
	BYTE		frameLength;		// bytes (m+n, per 802.15.4)  Does not count itself, 2 bytes of FCS, 1 of LQI, or 1 of RSSI.
	BYTE		frmCntrlLow;
	BYTE		frmCntrlHigh;
	BYTE		frameNumber;		// packet sequence number
	BYTE		dstPANid[2];
	BYTE		dstAddr[8];			// only 1st 2 bytes used if short addr
	BYTE		srcPANid[2];
	BYTE		srcAddr[8];
	BYTE		payloadSize;   		// length of payload field
	BYTEPNT		payload;			// points at payload start
	BYTE		lqi;				// LQI value for the received packet
	BYTE		rssi;
} Packet;

typedef struct
{
	BYTE		PANid[2];
	BYTE		addrShort[2];
	BYTE		addrLong[8];
	BYTE		channel;
	BYTE		seqNum;
	BYTE 		TX_busy;
	BYTE		TX_awatingAck;
	BYTE		TX_lastPackFail;
	BYTE		TX_busyChannel;
	
} Rd; 

Rd Radio; // Stores radio status
Packet Tx, Rx; // TX and RX buffers

// Control Frame defines
// Low Frame
#define PACKET_TYPE_BEACON     				0b0
#define PACKET_TYPE_DATA        			BIT0
#define PACKET_TYPE_ACK         			BIT1
#define PACKET_TYPE_COMMAND     			BIT0 | BIT1
#define PACKET_TYPE			     			BIT0 | BIT1 | BIT2
#define SECURITY_ENABLED					BIT3
#define SECURITY_DISABLED					0b0
#define SECURITY_FIELD						BIT3
#define ACK_REQUIRED_ENABLED				BIT5
#define ACK_REQUIRED_DISABLED				0b0
#define ACK_REQUIRED_FIELD 					BIT5
#define PAN_ID_COMP_ENABLED					BIT6
#define PAN_ID_COMP_DISABLED				0b0
#define PAN_ID_COMP_FIELD					BIT6	
// High Frame
#define SEQUENCE_NUM_SUP_ENABLED			BIT0 
#define SEQUENCE_NUM_SUP_DISABLED			0b0 
#define SEQUENCE_NUM_SUP_FIELD				BIT0
#define DST_NO_ADDR							0b0
#define DST_SHORT_ADDR						BIT3
#define DST_LONG_ADDR						BIT2 | BIT3
#define DST_ADDR_MODE						BIT2 | BIT3
#define SRC_NO_ADDR							0b0
#define SRC_SHORT_ADDR						BIT7
#define SRC_LONG_ADDR						BIT6 | BIT7
#define SRC_ADDR_MODE						BIT6 | BIT7

#endif
