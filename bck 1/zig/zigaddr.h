#ifndef ZIGADDR_H
#define ZIGADDR_H

// MRF24J40 hardware regsiter defintions - most of these names are from MiWi DE v3.1.3, and don't
// necessarily match the register names in the datasheet (I've fixed a few)

//long address registers


#define RFCTRL0 (0x200)
#define RFCTRL1 (0x201)
#define RFCTRL2 (0x202)
#define RFCTRL3 (0x203)
#define RFCTRL4 (0x204)
#define RFCTRL5 (0x205)
#define RFCTRL6 (0x206)
#define RFCTRL7 (0x207)
#define RFCTRL8 (0x208)
#define CAL1 (0x209)
#define CAL2 (0x20a)
#define CAL3 (0x20b)
#define SFCNTRH (0x20c)
#define SFCNTRM (0x20d)
#define SFCNTRL (0x20e)
#define RFSTATE (0x20f)
#define RSSI (0x210)
#define CLKIRQCR (0x211)
#define SRCADRMODE (0x212)
#define SRCADDR0 (0x213)
#define SRCADDR1 (0x214)
#define SRCADDR2 (0x215)
#define SRCADDR3 (0x216)
#define SRCADDR4 (0x217)
#define SRCADDR5 (0x218)
#define SRCADDR6 (0x219)
#define SRCADDR7 (0x21a)
#define RXFRAMESTATE (0x21b)
#define SECSTATUS (0x21c)
#define STCCMP (0x21d)
#define HLEN (0x21e)
#define FLEN (0x21f)
#define SCLKDIV (0x220)
//#define reserved (0x221)
#define WAKETIMEL (0x222)
#define WAKETIMEH (0x223)
#define TXREMCNTL (0x224)
#define TXREMCNTH (0x225)
#define TXMAINCNTL (0x226)
#define TXMAINCNTM (0x227)
#define TXMAINCNTH0 (0x228)
#define TXMAINCNTH1 (0x229)
#define RFMANUALCTRLEN (0x22a)
#define RFMANUALCTRL (0x22b)
#define RFRXCTRL RFMANUALCTRL
#define TxDACMANUALCTRL (0x22c)
#define RFMANUALCTRL2 (0x22d)
#define TESTRSSI (0x22e)
#define TESTMODE (0x22f)

#define NORMAL_TX_FIFO  (0x000)
#define BEACON_TX_FIFO  (0x080)
#define GTS1_TX_FIFO    (0x100)
#define GTS2_TX_FIFO    (0x180)
#define SECURITY_FIFO   (0x280)
#define RX_FIFO         (0x300)



/* Short Address Control Register Map */
#define RXMCR		0x00
#define PANIDL		0x01
#define PANIDH		0x02
#define	SADRL		0x03
#define SADRH		0x04
#define EADR0		0x05
#define EADR1		0x06
#define EADR2		0x07
#define EADR3		0x08
#define EADR4		0x09
#define EADR5		0x0A
#define EADR6		0x0B
#define EADR7		0x0C
#define RXFLUSH		0x0D

#define ORDER		0x10
#define TXMCR		0x11
#define ACKTMOUT	0x12
#define ESLOTG1		0x13
#define SYMTICKL	0x14
#define SYMTICKH	0x15
#define PACON0		0x16
#define PACON1		0x17
#define PACON2		0x18
#define TXBCON0		0x1A
#define TXNCON		0x1B
#define TXG1CON		0x1C
#define TXG2CON		0x1D
#define ESLOTG23	0x1E
#define ESLOTG45	0x1F

#define ESLOTG67	0x20
#define TXPEND		0x21
#define WAKECON		0x22
#define FRMOFFSET	0x23
#define TXSTAT		0x24
#define TXBCON1		0x25
#define GATECLK		0x26
#define TXTIME		0x27
#define HSYMTMRL	0x28
#define HSYMTMRH	0x29
#define SOFTRST		0x2A
#define SECCON0		0x2c
#define SECCON1		0x2d
#define TXSTBL		0x2e

#define RXSR		0x30
#define INTSTAT		0x31
#define INTCON		0x32
#define GPIO		0x33
#define TRISGPIO	0x34
#define SLPACK		0x35
#define RFCTL		0x36
#define SECCR2		0x37
#define BBREG0		0x38
#define BBREG1		0x39
#define BBREG2		0x3A
#define BBREG3		0x3B
#define BBREG4		0x3C
#define BBREG6		0x3E
#define CCAEDTH		0x3F



#endif
