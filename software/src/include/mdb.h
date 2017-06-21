/*
   Por Jardel Silveira (jardel.silveira@engelsolutions.com.br)
   Lib MDB para coin changer
   Criada em 12 de março de 2007
*/

#ifndef __MDB_H
#define __MDB_H

#include <engels.h>

void mdb_init (void);
void mdb_engine (void);

// Comandos aceitados pelo coin changer

#define RESET 					0x08
// NO VMC DATA
#define SETUP 					0x09
// NO VMC DATA
#define TUBESTATUS  			0x0A
// NO VMC DATA
#define POLL 					0x0B
// NO VMC DATA
#define COINTYPE 				0x0C
// 4 bytes VMC DATA
#define DISPENSE 				0x0D
// 1 byte VMC DATA
#define NOCOMMAND				0x0E
#define EXPANSIONCOMMAND 		0x0F

// Expansion Commands
#define IDENTIFICATION 			0x00
// NO VMC DATA
#define FEATUREENABLE 			0x01
// 4 bytes VMC DATA
#define SENDDIAGSTATUS			0x05
// NO VMC DATA

/*

#define PAYOUT					0x02
// 1 byte VMC DATA
#define PAYOUTSTATUS			0x03
// NO VMC DATA
#define PAYOUTVALUEPOLL			0x04
// NO VMC DATA
#define SENDCTRLMANFILLREP		0x06
// NO VMC DATA
#define SENDCTRLMANPAYREP		0x07
// NO VMC DATA
#define FTLREQTOCRV 			0xFA
// 5 bytes VMC DATA
#define FTLRETRYDENY			0xFB
// 3 bytes VMC DATA
#define FTLSENDBLOCK 			0xFC
// 33 bytes  VMC DATA
#define FTLOKTOSEND				0xFD
// 2 bytes VMC DATA
#define FTLREQTOSEND			0xFE
// 5 bytes VMC DATA*/
#define DIAGNOSTICS				0xFF
// n bytes VMC DATA

// Estados da mdb_engine
#define WAITMYADDRBYTE 		BIT0
#define WAITSUBCOMMAND   	BIT1
#define WAITDATA			BIT2
#define WAITCHK				BIT3
#define EXECUTE 			BIT4
#define ANSWER 				BIT5
#define WAITACK				BIT6
#define ERROR				BIT7
#define TIMEOUT 			0xFF

// Endereço do coin changer - somente 5 bits mais significativos
#define COINCHANGERADDR 	0x08

#define UNKNOWNCOMMAND 	100

#define ACK 					0x00
#define RET 					0xAA
#define NAK 					0xFF


	

#endif
