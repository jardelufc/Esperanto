/*
   Por Jardel Silveira (jardel.silveira@engelsolutions.com.br)
   Lib MDB para coin changer
   Criada em 12 de março de 2007
*/

#include <engels.h>
#include <serial.h>
#include <mdb.h>
#include <emp800.h>
#include <string.h>
#include <htc.h>
#include <timer.h>

// O define BONUSDATA permitirá o funcionamento em conjunto com o software simulador mdbmaster.exe 2.02 da bonus data AG
//#define __BONUSDATA__

// Calcula o CRC e monta array de resposta ao comando Answer to Setup
// Define o equipametno como nível 2 e configura os canais de 1-6 para moeda de 5,10,25,50,100 e 100 centavos, respectivamente
#define CHKANSWERTOSETUP 0x03+0x00+0x55+ 0x05+0x02+ 0x00+0x00+0x01+0x02+0x05+10+20+20+0x00+0x00+0x00+0x00+0x00+0x00+0x00+0x00+0x00+0x00
const BYTE  acAnswerToSetup[24]= {0x03, 0x00,0x55, 0x05,0x02, 0x00,0x00, 0x01,0x02,0x05,10,20,20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,CHKANSWERTOSETUP};

//Calcula o CRC e monta array de resposta ao comando de expansão Identification
#define CHKEXPID 'E'+ 'G'+'S'+ '1'+'2'+ '3'+'4'+'5'+'6'+'7'+'8'+'9'+'0'+'A'+'B'+'E'+'M'+'P'+'8'+'0'+'0'+'0'+'0'+'V'+'4'+ 'V'+ '5'+ 0x00+0x01+ 0x00+0x00+0x00+0x00
// Define MAN CODE como "EGS" (Engel Solutions), número de série para "1234567890AB" e modele e revisao para EMP80000V4V5, 
// revisao de soft para 0001 e informa que nao esta apto a responder outros comandos do nível 3
const BYTE  acAnswerToExpId[34]= {'E', 'G','S', '1','2', '3','4','5','6','7','8','9','0','A','B','E','M','P','8','0','0','0','0','V','4', 'V', '5', 0x00,0x01, 0x00,0x00,0x00,0x00,CHKEXPID};

// Define resposta ao SEND diag status informando que está tudo ok
#define CHKSENDDIAGSTATUS 0x01+0x00
const BYTE  acAnswerToSendDiagStatus[3]= {0x01,0x00,CHKSENDDIAGSTATUS};

// Define o número de máximo de tentativas de reenvio de resposta
#define MAXRETRIES 3
// Define o tempo em ms que o LED ficará aceso ou apagado em caso de estado de BLINK
#define MAXIDLETIME 250
// Define o tempo em ms entre dois bytes de um mesmo pacote
#define MAXTIMEBETWEENBYTES 2
#ifdef __BONUSDATA__
	// Define o tempo máximo de espera pelo BYTE de acknowledge
	#define MAXTIMETOACK 50
#else
	// Define o tempo máximo de espera pelo BYTE de acknowledge
	#define MAXTIMETOACK 8
#endif

// Variável de controle do estado da mdb_engine
static BYTE cState;
// Variável do comando, comprimento do vetor de dados de recepcao, variavel de cksum, array de dados de recepcao,
// ponteiro para dados 
static BYTE cCommand, cLengthData, cChkSum, acCommandData[10], *pData;
// Array de resposta, comprimento do array de resposta, numero de retentativas de envio, momento em que o 
// último byte foi recebido e variavel que define se recebeu comando de reset
static BYTE acMdbAnswer[17], cLength, cPointer, cRetries, cTimeLastCommand, cTimeLastByte, cReseted, cTimeout, cError, cIndexEE;

#define ERROR_NOCOMMAND 				0x01
#define ERROR_UNKNOWNCOMMAND 		0x02
#define ERROR_NINTHBIT					0x03
#define ERROR_EXCEEDMAXRETRIES		0x04
#define ERROR_EXPECTEDNAKAKORRET 	0x05

//#define ERRORLIB

#ifdef ERRORLIB
eeprom BYTE acErrors[EEPROM_SIZE];
#endif




// Função de execução do comando e preparo da resposta
int execute (void);

// Variável que define os canais de moedas (0-16 para cada canal) utilizados
const BYTE cCoinType[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

// Função que retorna o número de bytes esperado para cada comando
BYTE getLengthDataCommand (BYTE cCommand) {

switch (cCommand) {
   case RESET:
   case TUBESTATUS:
   case POLL:
   case SETUP:
   case IDENTIFICATION:
   case SENDDIAGSTATUS:
      return 0;
   break;
   case COINTYPE:
      return 4;
   break;
   case FEATUREENABLE:
	  return 4;
   break;
   case DISPENSE:
	  return 1;
#ifdef ERRORLIB
   case DIAGNOSTICS:
	  return 0;
#endif
   break;
   default: return UNKNOWNCOMMAND;
     break;
   }
}


// Inicializa máquina de estado MDB
void mdb_init (void) {
	// Define estado inicial da máquina MDB
	cState = WAITMYADDRBYTE;
	// Define que nao recebeu comando de reset
	cReseted = 0;
	cTimeout = 0;
#ifdef ERRORLIB
	cIndexEE = acErrors[0] ;
	if(cIndexEE==0xFF) {
		cIndexEE = 0x00;
		acErrors[0] = 0x00;
	}
#endif
}

// Executa a máquina de estados mdb
void mdb_engine (void) {

   static BYTE cData;

	switch (cState) {
   	case WAITMYADDRBYTE:
   		// No caso de ter sido resetado, verifica se nao ficou IDLE por tempo maximo
			if(cReseted==1 && cCommand != RESET && (!cTimeout)) {
				if( ElapsedTime(cTimeLastCommand) > MAXIDLETIME) {
					// Desabilita a leitora de moedas
					emp800_disableAccept();			
					cTimeout = 1;
				}
			}
			// Caso tenha chegado algum BYTE
			if(RCIF) {
				// Lê o byte recebido
         			cData = RCREG;
         			// Casa o nono bit esteja setado
				if(RCSTA & 0x01) {
					// Caso o comando esteja enderecado a mim
					if( ((cData & 0xF8)==COINCHANGERADDR)) {
						// Caso seja um comando desconhecido
						if (cData == NOCOMMAND)  {
							// Vai para o estado de erro
							cState = ERROR;
							cError = ERROR_NOCOMMAND;
						}
						// Caso seja um comando válido
						else {
							// Copia dado para variável de comando
							cCommand = cData;
							// Caso esteja resetado (aceita qq comando) ou seja um comando de reset
							if(cReseted==1 || cCommand==RESET) {
								// Vai para o estado do espera por subcomando
								cState = WAITSUBCOMMAND;
								// Inicializa o chksum, o qual inclui todos os bytes recebdos (comando, subcomando e dado)
								cChkSum = cCommand;
								// Inicializa contador de tempo inter-byte
								cTimeLastByte = getTimeCounter ();
								if(cTimeout) {
									emp800_enableAccept();
									cTimeout = 0;
									//LedOn();
								}
							}
						}
					}
				}
      	} // end if(RCIF)
			break;
		// Estado de espera por sub comando
		case WAITSUBCOMMAND:
			// Espera sub comando, caso seja um comando extendido
			if(cCommand == EXPANSIONCOMMAND) {
				// Caso tenha recebido algum BYTE ...
				if(RCIF) {
					// Zera contador de tempo
					cTimeLastByte = getTimeCounter ();
					//Lê o byte
					cData = RCREG;
					// Caso o nono bit esteja resetado ...
					if(!(RCSTA & 0x01)) {
						// Recebe sub-comando
						cCommand = cData;
						// Lê comprimento esperado do dado
						cLengthData = getLengthDataCommand (cCommand);
						// Atualiza checksum
						cChkSum += cData;	
						// Verifica se é um comando desconhecido					
					 	if (cLengthData==UNKNOWNCOMMAND)  {
							cState = ERROR;
							cError = ERROR_UNKNOWNCOMMAND;
						}
						// Caso seja um comando conhecido, vai para o estado de espera de dados
						// e inicializa ponteiro de rececpcao de dados
						else {
							cState = WAITDATA;
							pData = acCommandData;
						}
					}
					// Caso o nono bit esteja setado ...
					else {
						cState = ERROR;
						cState = ERROR_NINTHBIT;
					}
				}
				// Se não recebeu nenhum byte, verifica se excedeu tempo máximo entre bytes
				else if(ElapsedTime(cTimeLastByte) > MAXTIMEBETWEENBYTES)
					cState = TIMEOUT;
			}
			// Se não é um comando expandido, vai para o estado de recepcao dos dados
			else {
					cState = WAITDATA;
					pData = acCommandData;
					cLengthData = getLengthDataCommand (cCommand);
			 		if (cLengthData==UNKNOWNCOMMAND)  {
						cState = ERROR;
						cError = ERROR_UNKNOWNCOMMAND;
					}
			}
			break;
      case WAITDATA:
      	// Caso existam dados a serem recebidos
			if(cLengthData) {
				// Caso tenha chegado algum byte 
				if(RCIF) {
					// Zera contador de tempo inter-byte
					cTimeLastByte = getTimeCounter ();
					// Copia dado recebido para o array de recepcao de dados
					*pData = RCREG;
					// Caso o nono BIT não esteja setado
					if(!(RCSTA & 0x01)) {
						// Decrementa contador do tamanho esperado
						cLengthData--;
						// Atualiza checksum
						cChkSum += *pData;
						// Incrementa ponteiro de dados de recepcao
						pData++;
					}
					// Caso nono BIT nao esteja setado
					else {
						cState = ERROR;
						cError = ERROR_NINTHBIT;
					}		
				}
				// Caso não tenham chegad nenhum byte, verifica se não excedeu tempo máximo entre bytes
				else if(ElapsedTime(cTimeLastByte) > MAXTIMEBETWEENBYTES)
					// Vai para o estado de TIMEOUT
					cState = TIMEOUT;
			}
			// Caso todos BYTES esperados já tenham sido recebidos
			else {
				// Vai parao estado de espera por byte de CHK
				cState = WAITCHK;
				// Inicializa contador de tempo entre bytes
				cTimeLastByte = getTimeCounter ();
			}
			break;
		// Estado de espera do BYTE de CHKSUM
		case WAITCHK:
			// Caso tenha recebido algum byte
			if(RCIF) {
				// Recebe o dado
				cData = RCREG;
				// Verifica se nono bit não está setado
				if(!(RCSTA & 0x01)) {
					// Caso o chksum confira
					if(cData ==	cChkSum) {
						// vai para o estado de EXECUTE
						cState = EXECUTE;
					}
					// Se o chksum não confere, vai para o estado de ANSWER
					else {	
						// Vai para o estado de resposta
						cState = ANSWER;					
						// Seta comprimento da resposta para 1 byte
						cLength = 1;
						// Inicializa ponteiro da resposta
						cPointer = 0;
						// Responde NAK visto que nao era esperado o nono bit setado
						acMdbAnswer[0] = NAK;
					}		
				}
				// Caso nono BIT esteja setado
				else {
					// Vai para o estado de erro
					cState = ERROR;
					cError = ERROR_NINTHBIT;
					break;
				}
			}
			// Se não recebeu nenhum byte, verifica se não excedeu tempo máximo entre bytes
			else if(ElapsedTime(cTimeLastByte) > MAXTIMEBETWEENBYTES)
				// Vai para o estado de timeout
				cState = TIMEOUT;
			break;
		// Estado de execucao de comando
      case EXECUTE:
      	// Executa o comando e monta a resposta
      	execute();
      	// Vai para o estado de respost
	     	cState = ANSWER;
	     	// Inicializa ponteiro da resposta
		 	cPointer = 0;
		 	// Inicializa contador de retentativas
		 	cRetries = 0;
	     	break;
	   // Estado de resposta
      case ANSWER:
      	// Verifica se pode transmitir outro byte
			if(TXIF) {	
				// Incrementa ponteiro de resposa
				cPointer++;
				// Verifica se este é o último BYTE a ser enviado
				if (cPointer == cLength) {
					// Verifica se deve esperar por byte de ack
					if (cLength>1) {
						// Vai para o estado de wait ack
						cState=WAITACK;
						// Inicializa contador de tempo por resposta de ACK
						cTimeLastByte = getTimeCounter ();
					}
					// Caso não deva esperar por nenhum byte de ack
					else {							
						// Vai para o início da máquina esperar outro comando
						cState = WAITMYADDRBYTE;								     
						cTimeLastCommand = getTimeCounter ();
					}
					// Seta nono BIT
					#ifdef __BONUSDATA__
					TXSTA &= ~BIT0;
					#else
					TXSTA |= BIT0;
					#endif
				}
				// Reseta nono BIT caso nao seja último BYTE a ser enviado
				else {
					TXSTA &= ~BIT0;
 				}	
 				// Caso seja um resposta a um comando de setup
				if(cCommand == SETUP)
					TXREG = acAnswerToSetup[cPointer-1];
				// Caso seja resposta a um comando de IDENTIFICATION
				else if(cCommand == IDENTIFICATION)				
					TXREG = acAnswerToExpId[cPointer-1];	
				// Caso seja resposta a um comando de TUBESTATUS
				else if(cCommand == TUBESTATUS)
					TXREG = 0;
				// Caso seja resposta a um comando de SENDDIAGSTATUS
				else if(cCommand == SENDDIAGSTATUS)
					TXREG = acAnswerToSendDiagStatus[cPointer-1];	
#ifdef ERRORLIB
				// Caso seja resposta a um comando de DIAGNOSTICS
				else if(cCommand == DIAGNOSTICS)
					TXREG = acErrors[cPointer];	
#endif

				// Caso seja resposta a outro comando qualquer	
				else
					TXREG = acMdbAnswer[cPointer-1];
			}
			break;
      case WAITACK:
      	// Caso tenha chegado algum BYTE
			if(RCIF) {
				// Recebe o dado
				cData = RCREG;
				// Verifica se nono bit não está setado
				if(!(RCSTA & 0x01)) {			
					// Verifica se confere com o dado de ACK, já esperado
					if (cData== ACK) {
						// Inicializa contador de tempo inter-comandos
						cTimeLastCommand = getTimeCounter ();
						// Vai para o estado inicial a espera de BYTE de enderecamento
						cState = WAITMYADDRBYTE;
					}
					// Caso seja um BYTE de NAK ou RET
					else if(cData == NAK || cData==RET) {
						// Caso nao tenha excedido maximas re-tentativas
						if(cRetries < MAXRETRIES) {
							// Inicializa ponteiro de resposta
							cPointer =0 ;
							// Vai para o estado de resposta
							cState = ANSWER;
							// Incrementa contador de retentativas
							cRetries++;
						}
						// Caso tenha excedido maximas tentativas, vai para estado de ERROR
						else {
							cState = ERROR;
							cError= ERROR_EXCEEDMAXRETRIES;
						}
					}
					// Caso seja outro byte, vai para estado de ERRROR
					else  {
						cState = ERROR;
						cError = ERROR_EXPECTEDNAKAKORRET;
					}
				}// Caso o nono BIT esteja setado, vai para o estado de ERROR	
				else {
					cState = ERROR;
					cError = ERROR_NINTHBIT;
				}
			}
			// Caso nao tenha chegado BYTE, verifica se nao excedeu maximo tempo para responder
			else if(ElapsedTime(cTimeLastByte) > MAXTIMETOACK)
				cState = TIMEOUT;
			break;
		// Estado de ERROR
      case ERROR:
      	// Vai para o estado de espera por endereco
		cState = WAITMYADDRBYTE;
		#ifdef ERRORLIB
		if(cIndexEE<EEPROM_SIZE) { 
			acErrors[cIndexEE++] = cError; 
			acErrors[0] = cIndexEE;
		}
		#endif
			// Aciona o LED
			LedOn();
			// Captura contador de instante do último comando
			cTimeLastCommand = getTimeCounter();
			break;
		case TIMEOUT:
      	// Vai para o estado de espera por endereco
			cState = WAITMYADDRBYTE;
			// Aciona o LED
			LedOn();
			// Captura contador de instante do último comando
			cTimeLastCommand = getTimeCounter();
		break;
   }

}

int execute (void) {

	static BYTE cCoinsDeposited, i, j, cCheckSum;
   static  WORD wTemp;
	
	cCheckSum = 0;
  	
	switch(cCommand) {

     	case RESET:
			cLength =1;
    		acMdbAnswer[0] = ACK;
			emp800_disableAccept();
			cReseted = 1;
			break;
		case SETUP:
			cLength =24;
   	  	break;
     	case TUBESTATUS:
			cLength = 19;
     		break;
     	case POLL:
			// Zera contador de moedas enviadas
			cLength = 0;
			// Varre os seis tipos de moeda
			for (i=0;i<6;i++) {
				if(acCountCoin[i]) {
					acCountCoin[i]--;
					// Byte indicando o tipo de moeda depositada e que foi para o cashbox
					acMdbAnswer[0] = cCheckSum = (0x40 | cCoinType[i]);
					// Byte indicando que zero moedas estão no tubo desta moeda
					acMdbAnswer[1] = 0x00;
					// Incrementa contador de moedas enviadas
					cLength=2;
					break;
				}
			}
			// Caso não tenha sido enviada nenhuma moeda, envia byte de ACK
			if(cLength==0x00) {
     			acMdbAnswer[cLength] = ACK;
				cLength=1;
			}
			else {
				// Se não, envia o checksum
     				acMdbAnswer[cLength] = cCheckSum;
				cLength=3;
			}
      	break;
	case COINTYPE:
   	// Como eu não tenho controle sobre a aceitação por  tipos de moedas, vou aceitar todas as moedas
    	// se pelo menos um dos tipos listados para aceitar for um dos meus seis.
     	// emp800_enableAccept ();
     	emp800_disableAccept();
     	wTemp = acCommandData[0];
     	wTemp <<=8;
     	wTemp |= acCommandData[1];
		for(i=0;i<6;i++) {
			if( (wTemp >> cCoinType[i]) & 0x01 ) {
				emp800_enableAccept();
				break;
			}
		}
		acMdbAnswer[0] = ACK;
		cLength=1;
     	break;
	case DISPENSE:
	case FEATUREENABLE:
   	// O coin selector não pode ejetar moedas !!
		acMdbAnswer[0] = ACK;
		cLength=1;
      break;
	// EXPANSION COMMANDS
   case IDENTIFICATION:
		cLength = 34;
     	break;
   case SENDDIAGSTATUS:
		cLength = 3;
		break;
#ifdef ERRORLIB
   case DIAGNOSTICS:
	cLength = cIndexEE;
	acErrors[0] = 0x00;
	break;
#endif
/*
     case PAYOUT:
     break;

     case PAYOUTSTATUS:
     break;

     case PAYOUTVALUEPOLL:
     break;

     case SENDCTRLMANFILLREP:
     break;

     case FTLREQTOCRV:
     break;

     case FTLRETRYDENY:
     break;

     case FTLSENDBLOCK:
     break;

     case FTLOKTOSEND:
     break;

     case FTLREQTOSEND:
     break;

     case DIAGNOSTICS:
    break;*/

     }
}
