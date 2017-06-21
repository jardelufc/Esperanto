/*
   Por Jardel Silveira (jardel.silveira@engelsolutions.com.br)
   Lib MDB para coin changer
   Criada em 12 de mar�o de 2007
*/

#include <engels.h>
#include <mdb.h>
#include <emp800.h>
#include <serial.h>
#include <htc.h>
#include <pic.h>
#include <timer.h>
#include <stdlib.h>
#include <pic.h>


// Reseta o watchdog
void cleardog(void) {
	#asm
	clrwdt
	#endasm
}
	// Inicializa BITS de configuracao do PIC (fuses)
	__CONFIG (0x1E7C);
	// Oscillator: INTOSC IO ON RA6/RA7
	// Watchdog timer: On
	// Brown Out Detect: Enabled
	// Master clear enable: Enabled
	// Low Voltage Program: Disabled
	// Data EE Read Protect Enabled
	// Code protec: On


// Define se � uma vers�o de demonstra��o
//#define __DEMO__

#ifdef __DEMO__
	// Vari�vel n�o vol�til que conta o n�mero de boots efetuados pelo 
eeprom unsigned char runtimes = 0;
#endif

// Vari�vel auxiliar para opera��o na vari�vel armazenada na EEPROM
BYTE cAux;
void main (void) 
{
  	// Inicializa o TIMER 1 
	init_timer();
  	// Inicializa portos e vari�veis relacionados ao EMP 800
  	emp800_init();
  	// Inicializa porta serial para 9600 bps 9 bits e 1 stop bit
  	init_comms();
  	// Inicializa vari�veis relacionadas � interface MDB
  	mdb_init();


	#ifdef __DEMO__
	// L� da EEPROM para a RAM
  	cAux = runtimes;
	// Caso tenha excedido o n�mero m�ximo de boots permitidos
  	if(cAux==0) {
		// Coloca o LED para piscar
		LedBlink();
		while(1) {
			// Roda a m�quina de estados do LED
	    	LedEngine();
			// Reseta o WatchDog
	    	cleardog();
 		}
   }
	// L� da EEPROM para a RAM
   cAux = runtimes;
	// Decrementa vari�vel de 1 boot
   cAux--;
	// Escreve da RAM para a EEPROM
   runtimes = cAux;
#endif
   // Loop Infinito
   while(1) {
		// Roda m�quina de estados do protocolo MDB
      mdb_engine();
		// Roda m�quina de estados da interface paralela com o EMP800
      emp800_engine();
		// Roda m�quina de estados do LED
	#ifdef LEDLIB
      LedEngine();
	#endif
		// Resetao WatchDog
		cleardog();
   }
}
