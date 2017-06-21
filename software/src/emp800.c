/*
   Por Jardel Silveira (jardel.silveira@engelsolutions.com.br)
   Lib MDB para coin changer
   Criada em 24 de março de 2007
*/


#include <string.h>
#include <engels.h>
#include <emp800.h>
#include <htc.h>
#include <pic.h>
#include <timer.h>


// Variável contadoras de moedas recebidas e de moedas enviadas ao mdb master
BYTE acCountCoin[6];
// Variável para armazenamento do estado dos canais das moedas
static BYTE acStateCoin[6];
// Variáveis para controle da led engine
static BYTE cLed1, cLedState,cLedDiff;



// Inicialização dos ios e variáveis relativos ao moedeiro e relativo ao LED
void   emp800_init(void) {
	static BYTE cData;
	// Inicializa PORTA em modo de I/O
	cData = CMCON;
	cData |= 0x07;
	CMCON = cData;
	TRISA = 0xFD;   // 1111 1101
       // RA7 =  1 // OSC2 - UNUSED
       // RA6 =  1 // OSC1 - UNUSED
       // RA5 =  1 // MCLR - UNUSED
       // RA4 =  1 // COINOUTPUT3
       // RA3 =  1 // COINOUTPUT4
       // RA2 =  1 // COINOUTPUT2
       // RA1 =  0 // GENBLOCKING
       // RA0 =  1 // COINOUTPUT6
   TRISB = 0xEE;   // 1111 1010
       // RB7 =  1 // PGD - UNUSED
       // RB6 =  1 // PGC  - UNUSED
       // RB5 =  1 // COINOUTPUT1
       // RB4 =  0 // REJECT - UNUSED
       // RB3 =  1 // COINOUTPUT5
       // RB2 =  1 // TX
       // RB1 =  1 // RX
       // RB0 =  0 // LED
	emp800_disableAccept();// Default Doesn´t accept coins
   LED = 1;          // Default LED is off
	cLedState = 0;
	// Inicializa variável do estado das moedas
	memset(acStateCoin, WAITINGFALLING,6);
	//Inicializa variável de contagem das moedas
	memset(acCountCoin, 0x00,6);
}



void emp800_engine(void) {
	
	static BYTE cTimeCoin[6];
   static BYTE cCoinLine;
   static BYTE i;

   static BYTE cDataA, cDataB;
	//Lê o porto A
	cDataA = PORTA;
	//Lê o portoB
   cDataB = PORTB;
	// Inicializa cCoinLine com todos bits setados
	 cCoinLine = 0xFF;
	// Carrrega o estado do canal 1 no BIT0 de cCoinLine
	if(!(cDataB & BIT5)) 
		cCoinLine &= ~BIT0;
	// Carrrega o estado do canal 2 no BIT1 de cCoinLine
	if(!(cDataA & BIT2)) 
		cCoinLine &= ~BIT1;
	// Carrrega o estado do canal 3 no BIT2 de cCoinLine
	if(!(cDataA & BIT4)) 
		cCoinLine &= ~BIT2;
	// Carrrega o estado do canal 4 no BIT3 de cCoinLine
	if(!(cDataA & BIT3)) 
		cCoinLine &= ~BIT3;
	// Carrrega o estado do canal 5 no BIT4 de cCoinLine
	if(!(cDataB & BIT3)) 
		cCoinLine &= ~BIT4;
	// Carrrega o estado do canal 6 no BIT5 de cCoinLine
	if(!(cDataA & BIT0)) 
		cCoinLine &= ~BIT5;

// Captura o estado do BIT0 de cCoinLine
#define LINESTATE (cCoinLine&0x01)

	for (i =0; i < 6; i++) {
	   switch (acStateCoin[i]) {
      	// Aguarda descida do sinal que indica reconhecimento de moeda
         case WAITINGFALLING:
         	// Caso o sinal desça, muda o estado da máquina e zera contador de tempo
            if(!LINESTATE) {
            	acStateCoin[i] = WAITINGRISING;
               cTimeCoin[i] = getTimeCounter();
            }
         	break;
            // Aguarda subida do sinal após 100 ms
  			case WAITINGRISING:
         	// Verifica se o contador não excedeu o valor máximo
         	if(ElapsedTime(cTimeCoin[i]) > MAXTIMECOIN) {
         		acStateCoin[i] = WAITINGFALLING;
					//LedOn();
            	break;
         	}
         	// Verifica se o sinal subiu
         	if(LINESTATE) {
         		// Retorna ao estado default
            	acStateCoin[i] = WAITINGFALLING;
            	// Caso o sinal tenha ficado baixo por tempo mínimo, incrementa contador de moeda
            	if(ElapsedTime(cTimeCoin[i]) > MINTIMECOIN) 
            		acCountCoin[i]++;
				}
      		break;
		} //END SWITCH
		// Rola 1 bit para a esquerda de modo a pegar o estado do proximo canal
      cCoinLine >>= 1;
	} // END FOR
}
#ifdef LEDLIB
//Máquina de estados do LED
void LedEngine (void) {
	
	// No caso de o LED estar no estado de apagado, nao faz nada
	if(cLedState==0) 
		return;
	// Captura o tempo passado em ms desde de a última vez que o estado do LED mudou
	cLedDiff = ElapsedTime (cLed1);
	// Verifica se o tempo decorrido excede 250ms
	if(cLedDiff > 250) {
		// Caso o LED esteja em estado de ligado, apaga-o e muda para estado de apagado
		if(cLedState==1) {
      	LED = 1;
        	cLedState=0;
		}
		// Caso o LED esteja em estado de BLINK, inicializa novamente o contador de tempo e inverte a situacao atual do LED
		else if(cLedState==BLINK) {
			cLed1 = getTimeCounter();
			LED = !LED;
		}
	}
}
// Acende o LED, Muda seu estado para ligado e inicializar variavel contadora de tempo
void LedOn (void) {
   LED = 0;
   cLedState = 1;
   cLed1 = getTimeCounter () ;
}
// Coloca o LED em estado de BLINK e inicializa variável contadora de tempo
void LedBlink(void) {
	cLedState = BLINK;
	cLed1 = getTimeCounter();
}
#endif
