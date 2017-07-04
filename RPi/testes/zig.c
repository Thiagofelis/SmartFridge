#include "zig.h"

void zig_RstSoft ()
{
	zig_SetShort (SOFTRST, 0x07); // SOFTWARE RESET
	while ((zig_GetShort (SOFTRST) & 0x07) != 0x00);
	__delay_cycles (300); // n sei se precisa de td isso, olho dpois
}

void zig_RstRF ()
{
	zig_SetShort (RFCTL, 0x04);
	zig_SetShort (RFCTL, 0x00);
	
	__delay_cycles (100); // Falta fazer o calculo para esperar 192 μs
}

BYTE zig_GetShort (BYTE address)
{
	P2OUT &= ~BIT3;
	BYTE toReturn;
	SPI_Send ((address << 1) & 0b01111110);
	toReturn = SPI_Send (0x00);
	P2OUT |= BIT3;
	return toReturn;
}
	
void zig_SetShort (BYTE address, BYTE value)
{
	P2OUT &= ~BIT3;
	SPI_Send (((address << 1) & 0b01111111) | 0x01);
	SPI_Send (value);
	P2OUT |= BIT3;
}

BYTE zig_GetLong (WORD address)
{
	P2OUT &= ~BIT3;
	BYTE toReturn;
	SPI_Send (((address >> 3) & 0b01111111) | 0x80);
	SPI_Send (((address << 5) & 0b11100000));
	toReturn = SPI_Send (0x00);
	P2OUT |= BIT3;
	return toReturn;
}

void zig_SetLong (WORD address, BYTE value)
{	
	P2OUT &= ~BIT3;
	SPI_Send ((((BYTE)(address >> 3)) & 0b01111111) | 0x80);
	SPI_Send ((((BYTE)(address << 5)) & 0b11100000) | 0x10);
	SPI_Send (value);
	P2OUT |= BIT3;
}

void zig_SelChannel (BYTE channel)
{
	zig_SetLong (RFCTRL0, ((channel - 11) << 4) | 0x03);
	zig_RstRF ();
}
/*
void zig_SetPANID (WORD PANID)
{
	BYTE PL = PANID & 0xff, PH = (PANID & 0xff00) >> 8;
	zig_SetShort (PANIDL, PL);
	zig_SetShort (PANIDH, PH);
}
	
void zig_SetShortDevAddr (WORD ShortAddr)
{
	BYTE SAL = ShortAddr & 0xff, SAH = (ShortAddr & 0xff00) >> 8;
	zig_SetShort (SADRL, SAL);
	zig_SetShort (SADRH, SAH);
}
void zig_SetLongDevAddr (WORD LongAddr[4])
{
	BYTE LA[8];
	BYTE i;
	for (i = 0; i < (BYTE) 4; i++)
	{
		LA[i] = LongAddr[i] & 0xff;
		LA[i + 1] = (LongAddr[i] & 0xff00) >> 8;
	}
	for (i = 0; i < (BYTE) 8; i++)
	{
		zig_SetShort (EADR0 + i, LA[i]);	
	}
}

void zig_BuildTXPackage (BYTEPNT *TxPnt)
{
	Tx.frameType = PACKET_TYPE_DATA;
	Tx.securityEnabled = 0;
	Tx.framePending = 0;
	Tx.ackRequest = 1;
	Tx.panIDcomp = 1;
	Tx.dstAddrMode = SHORT_ADDR_FIELD;
	Tx.frameVersion = 0;
	Tx.srcAddrMode = NO_ADDR_FIELD;
	Tx.dstPANID = RadioStatus.MyPANID;
	Tx.dstAddr = RadioStatus.MyShortAddress;
	Tx.payload = txPayload;
	
	
	
}
*/

WORD zig_ContiguousWrite (WORD addr, BYTEPNT mem, int count)
{
	int i;	
	for (i = 0; i < count; i++)
	{
		zig_SetLong (addr++, mem[i]);
	}

	return addr;
}

WORD zig_ContiguousRead (WORD addr, BYTEPNT mem, int count)
{
	int i;	
	for (i = 0; i < count; i++)
	{
		mem[i] = zig_GetLong (addr++);
	}

	return addr;
}



void zig_TX_Transmit () // funcionando :)
{

//	while (Radio.TX_busy == 1);	
	Blink (GREEN);
	WORD currAddr = 0x02; // jump Header and Frame lenght, fill later
	
	// Write control frame
	currAddr = zig_ContiguousWrite (currAddr, &(Tx.frmCntrlLow), 1);
	currAddr = zig_ContiguousWrite (currAddr, &(Tx.frmCntrlHigh), 1);
	
	currAddr = zig_ContiguousWrite (currAddr, &(Radio.seqNum), 1);
	
	Radio.seqNum = (Radio.seqNum != 0xff) ? (Radio.seqNum + 1) : 0x80; // seqNum always >=0x80, loops from 0xff to 0x80
 	
	if ((Tx.frmCntrlHigh & DST_ADDR_MODE) == DST_SHORT_ADDR) 
	{	
		currAddr = zig_ContiguousWrite (currAddr, Tx.dstPANid, 2);
		currAddr = zig_ContiguousWrite (currAddr, Tx.dstAddr, 2);
	}
	else if ((Tx.frmCntrlHigh & DST_ADDR_MODE) == DST_LONG_ADDR) 
	{
		currAddr = zig_ContiguousWrite (currAddr, Tx.dstPANid, 2);
		currAddr = zig_ContiguousWrite (currAddr, Tx.dstAddr, 8);
	}
	
	if ( ((Tx.frmCntrlHigh & DST_ADDR_MODE) != DST_NO_ADDR) 
	  && ((Tx.frmCntrlHigh & SRC_ADDR_MODE) != SRC_NO_ADDR)  
	  && ((Tx.frmCntrlLow & PAN_ID_COMP_FIELD) == PAN_ID_COMP_DISABLED)) 
	{
		currAddr = zig_ContiguousWrite (currAddr, Radio.PANid, 2);
	}
	
	if ((Tx.frmCntrlHigh & SRC_ADDR_MODE) == SRC_SHORT_ADDR) 
	{
		currAddr = zig_ContiguousWrite (currAddr, Radio.addrShort, 2);
	} 
	else if ((Tx.frmCntrlHigh & SRC_ADDR_MODE) == SRC_LONG_ADDR) 
	{
		currAddr = zig_ContiguousWrite (currAddr, Radio.addrLong, 8);
	}
	
	zig_SetLong (0x00, currAddr - 2); // minus the lenght of the first 2 bytes
	currAddr = zig_ContiguousWrite (currAddr, Tx.payload, Tx.payloadSize);
	zig_SetLong (0x01, currAddr - 2);

	zig_SetShort (TXNCON, ((Tx.frmCntrlLow & ACK_REQUIRED_FIELD) == ACK_REQUIRED_ENABLED) ? (BIT0 | BIT2) : BIT0); // start transmission

	if ((Tx.frmCntrlLow & ACK_REQUIRED_FIELD) == ACK_REQUIRED_ENABLED)
	{
		Radio.TX_awatingAck = 1;
	}
	Radio.TX_busy = 1;
}

int zig_RX_Receive ()
{
	zig_SetShort (BBREG1, BIT2); // stop rx fifo from receive packets
	
	WORD currAddr = 0x300;
	//currAddr = zig_ContigousRead (currAddr, buffer, 4); // gets frame length (1), frame control (2), frame number (1)
	
	currAddr = zig_ContiguousRead (currAddr, &(Rx.frameLength), 1);
	currAddr = zig_ContiguousRead (currAddr, &(Rx.frmCntrlLow), 1);
	currAddr = zig_ContiguousRead (currAddr, &(Rx.frmCntrlHigh), 1);
	currAddr = zig_ContiguousRead (currAddr, &(Rx.frameNumber), 1);
	
	if ((Rx.frmCntrlLow & PACKET_TYPE) != PACKET_TYPE_ACK) // not an ack frame
	{
		currAddr = zig_ContiguousRead (currAddr, &(Rx.dstPANid[0]), 2); // gets PANID (2)		
	}
	
	if ((Rx.frmCntrlLow & SECURITY_FIELD) == SECURITY_ENABLED) // security enable, not implemented
	{
		return -1;
	}

	if ((Rx.frmCntrlHigh & DST_ADDR_MODE) == DST_SHORT_ADDR) // dst short mode	
	{
		currAddr = zig_ContiguousRead (currAddr, &(Rx.dstAddr[0]), 2);
	}
	else if ((Rx.frmCntrlHigh & DST_ADDR_MODE) == DST_LONG_ADDR) // dst long mode
	{
		currAddr = zig_ContiguousRead (currAddr, &(Rx.dstAddr[0]), 8);
	}
	
	/*if ( (Rx.frmCntrlHigh & 0b00110000) && (buffstart[2] & 0b00000011) && !(buffstart[2] & 0b00000010) ) // src PANID
	{
		currAddr = zig_ContigousRead (currAddr, buffer, 2);
	}*/
	
	
	if ( ((Rx.frmCntrlHigh & DST_ADDR_MODE) != DST_NO_ADDR) 
	  && ((Rx.frmCntrlHigh & SRC_ADDR_MODE) != SRC_NO_ADDR)  
	  && ((Rx.frmCntrlLow & PAN_ID_COMP_FIELD) == PAN_ID_COMP_DISABLED))
	{
		currAddr = zig_ContiguousRead (currAddr, &(Rx.srcPANid[0]), 2);	
	}
		
	if ((Rx.frmCntrlHigh & SRC_ADDR_MODE) == SRC_SHORT_ADDR) // dst short mode	
	{
		currAddr = zig_ContiguousRead (currAddr, &(Rx.srcAddr[0]), 2);
	}
	else if ((Rx.frmCntrlHigh & SRC_ADDR_MODE) == SRC_LONG_ADDR) // dst long mode
	{
		currAddr = zig_ContiguousRead (currAddr, &(Rx.srcAddr[0]), 8);
	}
	/*
	
	BYTE PayloadSize = buffstart[0] - (currAddr - 1) - 2;
	
	BYTEPNT PayloadPnt = buffer;
	
	currAddr = zig_ContigousRead (currAddr, buffer, PayloadSize);
	
	strncpy (rxPayload, PayloadPnt, PayloadSize);
	
	rxPayload[PayloadSize] = '\0';*/
}
/*
void zig_CheckInterrupt ()
{
	if (!(P2OUT & INTPIN)) // define INTPIN as the pin connected do INT for this to work
	{
		return;
	}
	
	BYTE interrupt = zig_GetShort (INSTAT);
	
	if (interrupt & BIT3) // RX interrupt
	{
		//zig_RX ();
	}
}*/

void zig_Init (BYTE channel, BYTE srcAddrLong[], BYTE srcAddrShort[], BYTE srcPANid[])
{
	zig_RstSoft ();
	zig_SetShort (RXFLUSH, 0x01);
	zig_SetShort (PACON2, 0x98);
	zig_SetShort (TXSTBL, 0x95);
	zig_SetLong (RFCTRL0, 0x03);
	zig_SetLong (RFCTRL1, 0x02);
	zig_SetLong (RFCTRL2, 0x80);
	zig_SetLong (RFCTRL6, 0x90);
	zig_SetLong (RFCTRL7, 0x80);
	zig_SetLong (RFCTRL8, 0x10);
	zig_SetLong (SCLKDIV, 0x21);
	zig_SetShort (BBREG2, 0x80);
	zig_SetShort (CCAEDTH, 0x60);
	zig_SetShort (BBREG6, 0x40);
	zig_SetShort (INTCON, 0b11110110); // ativa int de rx e tx
	zig_SelChannel (channel); // Seleciona canal 11
	zig_RstRF ();
	zig_SetShort (RXMCR, 0x00); 
	zig_SetLong (RFCTRL3, 0x00); // Seleciona potencia maxima
	zig_GetShort (INTSTAT); // Limpa int pendentes
	zig_SetShort (BBREG1, 0x00);
	zig_configRadioAddr (srcAddrLong, srcAddrShort, srcPANid); 
	zig_RstRF ();
	
	Radio.channel = channel;
	Radio.seqNum = 0x80;
	Radio.TX_busy = 0;
	Radio.TX_awatingAck = 0;
	Radio.TX_lastPackFail = 0;
	Radio.TX_busyChannel = 0;
}

void zig_TX_config (BYTE frameType, BYTE ackRequired, BYTE PANcomp, 
				BYTE noSequenceNum, BYTE dstAddrMode, BYTE srcAddrMode)
{ // Security always disabled, frame version always 00, frame pending always disabled, no IEs
	
	Tx.frmCntrlLow = frameType | ackRequired | PANcomp;
	Tx.frmCntrlHigh = noSequenceNum | dstAddrMode | srcAddrMode; 
}

void zig_configRadioAddr (BYTE srcAddrLong[], BYTE srcAddrShort[], BYTE srcPANid[])
{ 
	int i;
	for (i = 0; i < 8; i++)
	{
		Radio.addrLong[i] = srcAddrLong[i];
	}
	for (i = 0; i < 2; i++)
	{
		Radio.addrShort[i] = srcAddrShort[i];
		Radio.PANid[i] = srcPANid[i];
	}
	
	zig_SetShort (PANIDL, srcPANid[0]);
	zig_SetShort (PANIDH, srcPANid[1]);
			
	zig_SetShort (SADRL, srcAddrShort[0]);
	zig_SetShort (SADRH, srcAddrShort[1]);
				  
	for (i = 0; i < 8; i++)
	{
		zig_SetShort (EADR0 + i, srcAddrLong[i]);
	}
}


void zig_TX_configDstAddr (BYTE dstAddr[], BYTE dstPANid[])
{ //ALWAYS set the addresses after zig_TX_config
	int i;
	
	for (i = 0; i < 2; i++)
	{
		Tx.dstPANid[i] = dstPANid[i];
	}
	
	
	if ((Tx.frmCntrlHigh & DST_ADDR_MODE) == DST_LONG_ADDR)
	{
		for (i = 0; i < 8; i++)
		{
			Tx.dstAddr[i] = dstAddr[i];
		}
	}
	else if ((Tx.frmCntrlHigh & DST_ADDR_MODE) == DST_SHORT_ADDR)
	{
		for (i = 0; i < 2; i++)
		{
			Tx.dstAddr[i] = dstAddr[i];
		}
	}
}

void zig_TX_PayloadToBuffer (BYTEPNT payload, BYTE payloadSize)
{
	Tx.payload = payload;
	Tx.payloadSize = payloadSize;
}
 

#pragma vector = PORT2_VECTOR
__interrupt void Port2 (void) // RX/TX Interrupt routine
{	
	//Blink (RED); // Blinks sao testes
	BYTE intLog = zig_GetShort (INTSTAT);
	if (intLog & BIT0) // TX interruption
	{
		Blink (RED);
		Radio.TX_busy = 0;
		if (Radio.TX_awatingAck)
		{
	//		Blink (RED);
			Radio.TX_awatingAck = 0;
			BYTE txLog = zig_GetShort (TXSTAT);
			if (txLog & BIT0) // not successful?
			{
	//			Blink (GREEN);
				Radio.TX_lastPackFail = 1;
				if (txLog & BIT5) // fail due to busy channel?
				{
					Radio.TX_busyChannel = 1;
				}
				else
				{	//Blink (GREEN);
					Radio.TX_busyChannel = 0;
				}
			}
			else
			{	//Blink (RED);
				Radio.TX_lastPackFail = 0;
			}
		}
	}
	if (intLog & BIT3) // RX interruption
	{
		// calls interrupt routine from other module
		Blink (GREEN); //temporario, so para testes
		//BlinkBinary (zig_GetLong (0x30e));
	}
	P2IFG &= ~BIT1;
}
