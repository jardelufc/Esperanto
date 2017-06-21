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

// retorna o tempo decorrido desde a �ltima vez que foi executada a macro getimecounter
// Esta funcao soh funciona caso ElapsedTime seja executada no m�ximo 250ms ap�s getTimeCounter
BYTE ElapsedTime (BYTE count1) {
	// Vari�vel para leiturado tempo atual - a��o at�mica !
	static BYTE count2;
	// A��o at�mica !
	count2 = miliseconds;
	// Verifica se o contador j� virou desde a �ltima vez que gettimecounter foi chamada
	if(count2 >= count1)
		return (count2 - count1);
	else
		return (255-count1+count2);
}
