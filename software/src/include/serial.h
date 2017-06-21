/*
   Por Jardel Silveira (jardel.silveira@engelsolutions.com.br)
   Lib MDB para coin changer
   Criada em 24 de março de 2007
*/

#ifndef __SERIAL_H
#define __SERIAL_H

#include <engels.h>
#include <htc.h>
#include <pic.h>



#define BAUD 9600
#define FOSC 4000000L
#define NINE 1     /* Use 9bit communication? FALSE=8bit */

#define DIVIDER ((int)(FOSC/(16UL * BAUD) -1))
#define HIGH_SPEED 1

#if NINE == 1
#define NINE_BITS 0x40
#else
#define NINE_BITS 0
#endif

#if HIGH_SPEED == 1
#define SPEED 0x4
#else
#define SPEED 0
#endif

#if defined(_16F87) || defined(_16F88)
	#define RX_PIN TRISB2
	#define TX_PIN TRISB5
#else
	#define RX_PIN TRISC7
	#define TX_PIN TRISC6
#endif

/* Serial initialization */
#define init_comms()\
	SPBRG = DIVIDER;     	\
	RCSTA = (NINE_BITS|0x90);	\
	TXSTA = (SPEED|NINE_BITS|0x20)

// RX_PIN = 1;	
	// TX_PIN = 1;		  

/*void putch(unsigned char);
unsigned char getch(void);
unsigned char getche(void);*/


#endif
