/*
   Por Jardel Silveira (jardel.silveira@engelsolutions.com.br)
   Lib MDB para coin changer
   Criada em 24 de março de 2007
*/

#include <engels.h>
#include <serial.h>
#include <pic.h>
#include <stdio.h>
#include <htc.h>


void   serial_Init(void) {
	init_comms();
}
/*
BYTE   serial_getByte(void) {
	if(RCIF)
          return RCREG;	
}
BYTE serial_anyByte (void) {

}
 void 
putch(unsigned char byte) 
{
	
	while(!TXIF)	
		continue;
	TXREG = byte;
}

unsigned char 
getch() {
	
	while(!RCIF)
		continue;
	return RCREG;	
}

unsigned char getche(void)
{
	unsigned char c;
	putch(c = getch());
	return c;
}
*/
