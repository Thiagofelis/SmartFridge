#include "zig.h"

void Blink ()
{
	P1OUT ^= BIT0;
	__delay_cycles (200000);
	P1OUT ^= BIT0;
	__delay_cycles (200000);	
}

void zig_reset ()
{
	P2IE &= ~BIT1;
// Retirado o pino de reset, agr todos os rst sao soft
//	P2OUT ^= BIT2; // reseta com o pino
//	P2OUT ^= BIT2;
	App_delayMs (2);
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
	zig_SelChannel (Radio.channel); // Seleciona canal 11
	zig_RstRF ();
	zig_SetShort (RXMCR, 0x00); 
	zig_SetLong (RFCTRL3, 0x00); // Seleciona potencia maxima
	zig_GetShort (INTSTAT); // Limpa int pendentes
	
	zig_SetShort (PANIDL, Radio.PANid[0]);
	zig_SetShort (PANIDH, Radio.PANid[1]);		
	zig_SetShort (SADRL, Radio.addrShort[0]);
	zig_SetShort (SADRH, Radio.addrShort[1]);
	int i;
	for (i = 0; i < 8; i++)
	{
		zig_SetShort (EADR0 + i, Radio.addrLong[i]);
	}
	zig_RstRF ();
	
	Radio.TX_busy = false;
	Radio.TX_awatingAck = false;
	Radio.TX_lastPackFail = true;
	
	P2IE |= BIT1;
	zig_SetShort (BBREG1, 0x00);
}

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

WORD zig_ContiguousWrite (WORD addr, BYTEPNT mem, int count)
{
	int i;	
	for (i = 0; i < count; i++)
	{
		zig_SetLong (addr++, mem[i]);
	}

	return addr;
}

WORD zig_ContiguousRead (WORD addr, BYTE* mem, int count)
{
	int i;	
	for (i = 0; i < count; i++)
	{
		mem[i] = zig_GetLong (addr++);
	}

	return addr;
}

int zig_TX_Transmit () // funcionando :)
{
	
	if (Radio.TX_busy == true)
		return FAIL_BUSY;
	
	WORD currAddr = 0x02; // jump Header and Frame lenght, fill later
	
	// Write control frame
	currAddr = zig_ContiguousWrite (currAddr, &(Tx.frmCntrlLow), 1);
	currAddr = zig_ContiguousWrite (currAddr, &(Tx.frmCntrlHigh), 1);
	
	currAddr = zig_ContiguousWrite (currAddr, &(Tx.seqNum), 1);
	
	Tx.seqNum = (Tx.seqNum != 0xff) ? (Tx.seqNum + 1) : 0x80; // seqNum always >=0x80, loops from 0xff to 0x80
 	
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
	
	zig_SetLong (0x00, currAddr - 2); //HEADER LENGHT
	currAddr = zig_ContiguousWrite (currAddr, Tx.payload, Tx.payloadSize);
	zig_SetLong (0x01, currAddr - 2); //FRAME LENGHT

	zig_SetShort (TXNCON, ((Tx.frmCntrlLow & ACK_REQUIRED_FIELD) == ACK_REQUIRED_ENABLED) ? (BIT0 | BIT2) : BIT0); // start transmission

	if ((Tx.frmCntrlLow & ACK_REQUIRED_FIELD) == ACK_REQUIRED_ENABLED)
	{
		Radio.TX_awatingAck = true;
	}
	Radio.TX_busy = true; // tenho q implementar isso depois

	return 0;
}
/*
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
	
	//if ( (Rx.frmCntrlHigh & 0b00110000) && (buffstart[2] & 0b00000011) && !(buffstart[2] & 0b00000010) ) // src PANID
	//{
	//	currAddr = zig_ContigousRead (currAddr, buffer, 2);
	//}
	
	
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
	
	
	BYTE PayloadSize = buffstart[0] - (currAddr - 1) - 2;
	
	BYTEPNT PayloadPnt = buffer;
	
	currAddr = zig_ContigousRead (currAddr, buffer, PayloadSize);
	
	strncpy (rxPayload, PayloadPnt, PayloadSize);
	
	rxPayload[PayloadSize] = '\0';
	return 0;
}*/

int zig_RX_getLastPckt ()
{
	if (Radio.RX_empty == true)
	{
		return false;
	}

	Rx.frameLength = buffer[Radio.RX_buffFront][0];
	Rx.frmCntrlLow = buffer[Radio.RX_buffFront][1];
	Rx.frmCntrlHigh = buffer[Radio.RX_buffFront][2];
	Rx.seqNum = buffer[Radio.RX_buffFront][3];
	
	int currAddr = 0;
	if ((Rx.frmCntrlHigh & DST_ADDR_MODE) == DST_SHORT_ADDR) 
	{	
		//memcpy (&Rx.dstPANid[0], &buffer[Radio.RX_buffFront][4], 2);
		//memcpy (&Rx.dstAddr[0], &buffer[Radio.RX_buffFront][6], 2);
		currAddr = 8;
	}
	else if ((Rx.frmCntrlHigh & DST_ADDR_MODE) == DST_LONG_ADDR) 
	{
		//memcpy (&Rx.dstPANid[0], &buffer[Radio.RX_buffFront][4], 2);
		//memcpy (&Rx.dstAddr[0], &buffer[Radio.RX_buffFront][6], 8);
		currAddr = 14;
	}
	
	if ( ((Rx.frmCntrlHigh & DST_ADDR_MODE) != DST_NO_ADDR) 
	  && ((Rx.frmCntrlHigh & SRC_ADDR_MODE) != SRC_NO_ADDR)  
	  && ((Rx.frmCntrlLow & PAN_ID_COMP_FIELD) == PAN_ID_COMP_DISABLED)) 
	{
		memcpy (&Rx.srcPANid[0], &buffer[Radio.RX_buffFront][currAddr], 2);
		currAddr += 2;
	}
	
	if ((Tx.frmCntrlHigh & SRC_ADDR_MODE) == SRC_SHORT_ADDR) 
	{
		memcpy (&Rx.srcAddr[0], &buffer[Radio.RX_buffFront][currAddr], 2);
		currAddr += 2;
	} 
	else if ((Tx.frmCntrlHigh & SRC_ADDR_MODE) == SRC_LONG_ADDR) 
	{
		memcpy (&Rx.srcAddr[0], &buffer[Radio.RX_buffFront][currAddr], 8);
		currAddr += 8;
	}
	
	Rx.payloadSize = Rx.frameLength - currAddr + 0x301 - 2;
	memcpy (&Rx.payload[0], &buffer[Radio.RX_buffFront][currAddr], Rx.payloadSize);
	currAddr += Rx.payloadSize;
	
	memcpy (&Rx.fcs[0], &buffer[Radio.RX_buffFront][currAddr], 2);
	currAddr += 2;
	
	Rx.lqi =  buffer[Radio.RX_buffFront][currAddr];
	Rx.rssi =  buffer[Radio.RX_buffFront][currAddr + 1];
	
	Radio.RX_buffFront = (Radio.RX_buffFront + 1) % BUFFER_SIZE;
	
	if (Radio.RX_buffFront == Radio.RX_buffRear)
	{
		Radio.RX_empty = true;
	}
	return true;
}

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
	zig_configRadioAddr (srcAddrLong, srcAddrShort, srcPANid); 
	zig_RstRF ();
	
	Radio.channel = channel;
	Radio.TX_busy = false;
	Radio.TX_awatingAck = false;
	Radio.TX_lastPackFail = false;
	Radio.TX_busyChannel = false;
	Radio.RX_buffRear = 0;
	Radio.RX_buffFront = 0;
	Radio.RX_lastPackWasTruncated = false;
	Radio.RX_empty = true;
	Radio.RX_full = false;
	Radio.RX_lastPackWasIgnored = false;
	
	Tx.seqNum = 0x80;
	zig_SetShort (BBREG1, 0x00);
	P2IE |= BIT1;
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

void zig_TX_PayloadToBuffer (BYTE* payload, BYTE payloadSize)
{
	Tx.payload = payload;
	Tx.payloadSize = payloadSize;
}
 
/*
#pragma vector = PORT2_VECTOR
__interrupt void Port2 (void) // RX/TX Interrupt routine
{	
	
	if (P2IFG & BIT1)
	{
		BYTE intLog = zig_GetShort (INTSTAT);
		if (intLog & BIT0) // TX interruption
		{
			P1OUT ^=BIT0;
			Radio.TX_busy = false;
			if (Radio.TX_awatingAck)
			{
				Radio.TX_awatingAck = false;
				BYTE txLog = zig_GetShort (TXSTAT);
				if (txLog & BIT0) // not successful?
				{		
					//P1OUT ^= BIT0;
					Radio.TX_lastPackFail = true;
					if (txLog & BIT5) // fail due to busy channel?
					{
						Radio.TX_busyChannel = true;
					}
					else
					{	
						Radio.TX_busyChannel = false;
					}
				}
				else
				{	
					Radio.TX_lastPackFail = false;
				}
			}
		}
		if (intLog & BIT3) // RX interruption
		{
			if (Radio.RX_full == true)
			{
				Radio.RX_lastPackWasIgnored = true;
			}
			else
			{
				zig_SetShort (BBREG1, BIT2); // stop rx fifo from receive packets
				buffer[Radio.RX_buffRear][0] = zig_GetLong (0x300); // == m + n + 2
				if ( (buffer[Radio.RX_buffRear][0] + 3) > MAX_RX_PACKET_SIZE)
				{
					Radio.RX_lastPackWasTruncated = true;
					zig_ContiguousRead (0x301, &buffer[Radio.RX_buffRear][1], MAX_RX_PACKET_SIZE - 1); 
				}
				else
				{
					zig_ContiguousRead (0x301, &buffer[Radio.RX_buffRear][1], buffer[Radio.RX_buffRear][0] + 2); 
				}
				Radio.RX_empty = false;
				Radio.RX_buffRear = (Radio.RX_buffRear + 1) % BUFFER_SIZE;
				
				if (Radio.RX_buffRear == Radio.RX_buffFront)
				{
					Radio.RX_full = true;
				}
				zig_SetShort (BBREG1, 0);
			}
		}
		P2IFG &= ~BIT1;
	}
}*/
