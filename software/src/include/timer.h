#ifndef __ENGELS_TIMER
#define __ENGELS_TIMER

#include <htc.h>
#include <pic.h>

void interrupt timer1_isr(void);
BYTE getTimeCounter (void) ;
BYTE ElapsedTime (BYTE count1);
extern BYTE	miliseconds;	// mili second count


#define RESETTIMER (0xFFFF-1000)

#define init_timer()\
	miliseconds = 0; \
	TMR1H = ((0xFF00|RESETTIMER)>>8);	\
	TMR1L =  (0x00FF|RESETTIMER);	\
	T1CON = 0b00000001;	\
	TMR1IE=1;	\
	PEIE=1;	\
	GIE=1; 
#define getTimeCounter() miliseconds

//#define ElapsedTime(count1)	((miliseconds>=count1)?(miliseconds-count1):(255-count1+miliseconds))


#endif
