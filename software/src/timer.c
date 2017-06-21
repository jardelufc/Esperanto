#include <pic.h>
#include <engels.h>
#include <timer.h>

/*
 *	Calculate preload value for one second timer
 */


BYTE	miliseconds;	// mili second count

BYTE count2;

/* service routine for timer  interrupt */
void interrupt
timer1_isr(void)
{
//	di();
	if(TMR1IE && TMR1IF) {
		TMR1H = ( (0xFF00 & RESETTIMER) >> 8 );
		TMR1L =  (0x00FF & RESETTIMER) ;
		miliseconds++;
		TMR1IF=0;
	}
//	ei();
}

// retorna o tempo decorrido desde a última vez que foi executada a macro getimecounter
// Esta funcao soh funciona caso ElapsedTime seja executada no máximo 250ms após getTimeCounter
BYTE ElapsedTime (BYTE count1) {
	// Variável para leiturado tempo atual - ação atômica !
	static BYTE count2;
	// Ação atômica !
	count2 = miliseconds;
	// Verifica se o contador já virou desde a última vez que gettimecounter foi chamada
	if(count2 >= count1)
		return (count2 - count1);
	else
		return (255-count1+count2);
}
