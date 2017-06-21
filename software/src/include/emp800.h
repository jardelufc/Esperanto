/*
   Por Jardel Silveira (jardel.silveira@engelsolutions.com.br)
   Lib MDB para coin changer
   Criada em 24 de março de 2007
*/

#ifndef __EMP800_H
#define __EMP800_H

#include <engels.h>

extern BYTE acCountCoin[6];
#define WAITINGFALLING 0
#define WAITINGRISING 0xFF
#define MAXTIMECOIN 110
#define MINTIMECOIN 90


#define COINOUTPUT1 RB5 // input
#define COINOUTPUT2 RA2 // input - 10 centavos de real - pino 8
#define COINOUTPUT3 RA4 // input
#define COINOUTPUT4 RA3 // input
#define COINOUTPUT5 RB3 // input
#define COINOUTPUT6 RA0 // input

#define GENBLOCKING RA1 // Ouput - active high
#define REJECT      RB4 // Ouput
#define LED         RB0 // Active low
#define BLINK 0xFF

#define emp800_enableAccept()\
	GENBLOCKING = 0;\
    REJECT      =1;

#define emp800_disableAccept()\
	GENBLOCKING = 1; \
	REJECT      = 0;



void   emp800_init(void);
void   emp800_engine(void);

void LedEngine (void);
void LedOn (void);
void LedBlink(void);

#endif
