//*************************************
//               MSP430F552x
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//    Vin0 -->|P6.0/CB0/A0      |
//    Vin1 -->|P6.1/CB1/A1      |
//    Vin2 -->|P6.2/CB2/A2      |
//    Vin3 -->|P6.3/CB3/A3      |
//    Vin3 -->|P6.4/CB4/A4      |
//
//*************************************

#include <msp430f5529.h>

#define row 6
#define col  5

// Motor 1
#define	M1CK	BIT6
#define M1ACK 	BIT2
#define M1ENAB	BIT6

// Motor 2
#define	M2CK	BIT7
#define M2ACK 	BIT2
#define M2ENAB	BIT4

// Motor 3
#define	M3CK	BIT6
#define M3ACK 	BIT1
#define M3ENAB	BIT3

// Motor 4
#define	M4CK	BIT0
#define M4ACK 	BIT7
#define M4ENAB	BIT2

// Motor 5
#define	M5CK	BIT2
#define M5ACK 	BIT4
#define M5ENAB	BIT0

/* Parametros */
unsigned int SetPoint[row][col] = {{2300, 2070, 1500, 2100, 2500},
                                   {1500, 1570, 700, 1500, 2500},
                                   {1500, 1570, 700, 1500, 1950},
                                   {2300, 2070, 1500, 2100, 1950},
                                   {2300, 2070, 1500, 2100, 2500},
                                   {2300, 2070, 1500, 2100, 1850}};

/* SetPoint Rules
   SetPoint[line][row] = {{M1_Step1, M2_Step1, M3_Step1, M4_Step1, M5_Step1},
						  {M1_Step2, M2_Step2, M3_Step2, M4_Step2, M5_Step2},
				   		  {M1_StepN, M2_StepN, M3_StepN, M4_StepN, M5_StepN},}; */

unsigned int Gap = 60;			// Gap para a Histerese do Motor
unsigned int SetTarget[5];			// Valor para Ajuste do Erro de Posição
unsigned int SetPointMax;		// Valor Máximo
unsigned int SetPointMin;		// Valor Minimo

/* Analog Values */
unsigned int Results_A0;
unsigned int Results_A1;
unsigned int Results_A2;
unsigned int Results_A3;
unsigned int Results_A4;

unsigned int i = 0;		        // indice para seleção do motor
unsigned int j = 0;    	 	        // indice do estado
unsigned int flagM1 = 0; 		// indice para controle de movimentação no estado do motor 1
unsigned int flagM2 = 0; 		// indice para controle de movimentação no estado do motor 2 
unsigned int flagM3 = 0; 		// indice para controle de movimentação no estado do motor 3 
unsigned int flagM4 = 0; 		// indice para controle de movimentação no estado do motor 4 
unsigned int flagM5 = 0; 		// indice para controle de movimentação no estado do motor 5 

/* Prototipos das Funções */
void Conf_timer(void);
void Conf_ADC(void);
void StateMachine(unsigned int SetPoint[row][col]);
void MotorCommand(unsigned int SetTarget[col], unsigned int Gap);

int main(void){

	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer
        
    // Configura Saídas
    P4DIR |= BIT7;                         // flag led
    //P4OUT |= BIT7;
    
    //Motor 1
    P6DIR |= M1CK;        // 
    P3DIR |= M1ACK;
    P1DIR |= M1ENAB;
    P6OUT &= ~M1CK;        // Forcar iniciar off
    P3OUT &= ~M1ACK;       // Forcar iniciar off
    P1OUT &= ~M1ENAB;      // Forcar iniciar off
    
        //Motor 2
    P2DIR |= M2CK;        // 
    P4DIR |= M2ACK;
    P4DIR |= M2ENAB;
    P2OUT &= ~M2CK;        // Forcar iniciar off
    P4OUT &= ~M2ACK;       // Forcar iniciar off
    P4OUT &= ~M2ENAB;      // Forcar iniciar off
    
        //Motor 3
    P2DIR |= M3CK;        // 
    P8DIR |= M3ACK;
    P2DIR |= M3ENAB;
    P2OUT &= ~M3CK;        // Forcar iniciar off
    P8OUT &= ~M3ACK;       // Forcar iniciar off
    P2OUT &= ~M3ENAB;      // Forcar iniciar off
    
        //Motor 4
    P4DIR |= M4CK;        // 
    P3DIR |= M4ACK;
    P8DIR |= M4ENAB;
    P4OUT &= ~M4CK;        // Forcar iniciar off
    P3OUT &= ~M4ACK;       // Forcar iniciar off
    P8OUT &= ~M4ENAB;      // Forcar iniciar off
    
        //Motor 5
    P2DIR |= M5CK;        // 
    P7DIR |= M5ACK;
    P2DIR |= M5ENAB;
    P2OUT &= ~M5CK;        // Forcar iniciar off
    P7OUT &= ~M5ACK;       // Forcar iniciar off
    P2OUT &= ~M5ENAB;      // Forcar iniciar off

    Conf_ADC();
    Conf_timer();
    
    __bis_SR_register(LPM0_bits + GIE);     // Enter LPM0, Enable interrupts
    
	while(1);

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void){

	P1OUT ^= 0x01;          // Sinaliza Interrupção
	ADC12CTL0 |= ADC12ENC + ADC12SC;
    /* ADC12ENC = Habilita a conversão,
       ADC12SC  = Inicia a conversão do ADC12 */

}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void){

        P4DIR |= BIT7;                         // flag led
	Results_A0 = ADC12MEM0;      // Move A0 results, IFG is cleared
	Results_A1 = ADC12MEM1;      // Move A1 results, IFG is cleared
	Results_A2 = ADC12MEM2;      // Move A2 results, IFG is cleared
	Results_A3 = ADC12MEM3;      // Move A3 results, IFG is cleared
	Results_A4 = ADC12MEM4;	     // Move A4 results, IFG is cleared 
        
    // Altera qual motor será controlado
        i++;
        
	if (i == col){
		i=0;
	}    

     StateMachine(SetPoint); //função controle da maquina de estados

}

// Config Timer
void Conf_timer(void){

	TA0CTL = TASSEL_2 + ID_2 + MC_1 + TACLR;
        /* SMCLK = fonte de clock do timer, MC_1 = contagem progressiva
           ou seja de 0 até o valor de TACCR0, TACLR = reseta o timer A,
	  contagem inicia em zero de novo */

	TA0CCTL0 |= CCIE;       // CCIE = habilita interrupção de comparação do canal
	TA0CCR0 = 0x0FFF;
        /* TA0CCR1 = registrador de captura e comparação, esta relacionado com o TACCR0
           Valor Máximo = 0xFFFE */
}

// Config. ADC
void Conf_ADC(void){

	P6SEL = 0x1F;           // Habilita canais de interrupção do A/D

	ADC12CTL0 = ADC12ON + ADC12MSC + ADC12SHT0_8;
    /*  ADC12ON = liga o ADC,
       ADC12MSC = seta multiplas conversões
    ADC12SHT0_8 = seleção de amostragem */
	
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_3 + ADC12SHS_0;
    /*   ADC12SHP = seleção de amostragem (definido pelos bits ADC12SHT0_8),
	ADC12CONSEQ_3 = Sequencia de canais, repetitivo, SHS_1 = disparo através do timer */

	ADC12MCTL0 |= ADC12INCH_0;              // 000 = referencia AVcc, ADC12INCH_0 = seleção do canal de entrada - A0
	ADC12MCTL1 |= ADC12INCH_1;              // A1
	ADC12MCTL2 |= ADC12INCH_2;              // A2
	ADC12MCTL3 |= ADC12INCH_3;	        // A3
	ADC12MCTL4 |= ADC12INCH_4 + ADC12EOS;   // A4, EOS = Esse canal é o ultimo da sequencia da conversão.

	ADC12IE = 0x08;         // Enable ADC12IFG.3

}

void StateMachine(unsigned int SetPoint[row][col]){

	if ( (flagM1 == 1) && (flagM2 == 1) && (flagM3 == 1) && (flagM4 == 1) && (flagM5 == 1) && (j < row) ){
		j++;
                flagM1 = 0;
                flagM2 = 0;
                flagM3 = 0;
                flagM4 = 0;
                flagM5 = 0;
                __delay_cycles(630000);
	}
        if ( j == row){
          j=0;
        }
        
	 SetTarget[i] = SetPoint[j][i]; // Seta posição destino do motor em questão.
	
	 MotorCommand(SetTarget, Gap); //lógica para acionamento do motor

}

void MotorCommand(unsigned int SetTarget[col], unsigned int Gap){

	SetPointMax = SetTarget[i] + Gap;   // Seta Valor Maximo
	SetPointMin = SetTarget[i] - Gap;   // Seta Valor Minimo
        if(i==4){
          SetPointMax = SetPointMax + 40;
          SetPointMin = SetPointMin - 40;
        }
	// Girar Sentido Horário

	switch(i){
	case 0: //LOGICA ACIONAMENTO MOTOR 1
		__delay_cycles(13000);
		if( Results_A0 > SetPointMax){ // Girar Sentido Antihorário
                        
			P6OUT |=  M1CK;        // ON
			P3OUT &= ~M1ACK;       // OFF
			P1OUT |=  M1ENAB;      // ENABLE

		}else if( Results_A0 < SetPointMin){ // Girar Sentido horário
                        P4OUT |= BIT7;
			P6OUT &= ~M1CK;        // OFF
			P3OUT |=  M1ACK;       // ON
			P1OUT |=  M1ENAB;      // ENABLE
                        
		}else if (flagM1!=1){ // Para o motor

			P6OUT &= ~M1CK;        // OFF
			P3OUT &= ~M1ACK;        // OFF
			P1OUT &= ~M1ENAB;       // DISABLE
		
			//__delay_cycles(5000000);  // delay

			flagM1=1;	//	Avisa que chegou na posição
		}//Aguardando outros motores
                break;
        case 1: //LOGICA ACIONAMENTO MOTOR 2
		__delay_cycles(13000);
		if( Results_A1 > SetPointMax){ // Girar Sentido Antihorário

			P2OUT |=  M2CK;        // ON
			P4OUT &= ~M2ACK;       // OFF
			P4OUT |=  M2ENAB;      // ENABLE
                        
		}else if( Results_A1 < SetPointMin){ // Girar Sentido horário
	
			P2OUT &= ~M2CK;        // OFF
			P4OUT |=  M2ACK;       // ON
			P4OUT |=  M2ENAB;      // ENABLE

			
		}else if (flagM2!=1){ // Parar o motor

			P2OUT &= ~M2CK;         // OFF
			P4OUT &= ~M2ACK;        // OFF
			P4OUT &= ~M2ENAB;       // DISABLE
		
			//__delay_cycles(500000);  // delay

			flagM2=1;	//	Avisa que chegou na posição
		} //Aguardando outros motores
		break;

	case 2: //LOGICA ACIONAMENTO MOTOR 3
                __delay_cycles(13000);
		if( Results_A2 > SetPointMax){ // Girar Sentido Antihorário

			P2OUT |=  M3CK;        // ON
			P8OUT &= ~M3ACK;       // OFF
			P2OUT |=  M3ENAB;      // ENABLE
			
		}else if( Results_A2 < SetPointMin){ // Girar Sentido horário
	
			P2OUT &= ~M3CK;        // OFF
			P8OUT |=  M3ACK;       // ON
			P2OUT |=  M3ENAB;      // ENABLE
                        
		}else if (flagM3!=1){ // Manter Parado
                        
                        P2OUT &= ~M3CK;        // Forcar iniciar off
                        P8OUT &= ~M3ACK;       // Forcar iniciar off
                        P2OUT &= ~M3ENAB;      // Forcar iniciar off
		
			//__delay_cycles(500000);  // delay

			flagM3=1;	//	Avisa que chegou na posição
		} //Aguardando outros motores
		break;
        
	case 3: //LOGICA ACIONAMENTO MOTOR 4
                
//                __delay_cycles(500000);	
//		if( Results_A3 > SetPointMax){ // Girar Sentido Antihorário
//
//			P4OUT |=  M4CK;        // ON
//			P3OUT &= ~M4ACK;       // OFF
//			P8OUT |=  M4ENAB;      // ENABLE
//			
//		}else if( Results_A3 < SetPointMin){ // Girar Sentido horário
//	
//			P4OUT &= ~M4CK;        // OFF
//			P3OUT |=  M4ACK;       // ON
//			P8OUT |=  M4ENAB;      // ENABLE
//
//		}else if (flagM4!=1){ //Para o motor
//
//			P4OUT &= ~M4CK;         // OFF
//			P3OUT &= ~M4ACK;        // OFF
//			P8OUT &= ~M4ENAB;       // DISABLE
//		
//			__delay_cycles(500000);  // delay
//
			flagM4=1;	//	Avisa que chegou na posição
//		} //Aguardando outros motores
		break;
                
	case 4: //LOGICA ACIONAMENTO MOTOR 5
                          SetPointMax = SetPointMax + 20;
                SetPointMin = SetPointMin - 50;
                __delay_cycles(13000);
		if( Results_A4 > SetPointMax){ // Girar Sentido Antihorário

			P2OUT |=  M5CK;        // ON
			P7OUT &= ~M5ACK;       // OFF
			P2OUT |=  M5ENAB;      // ENABLE
			
		}else if( Results_A4 < SetPointMin){ // Girar Sentido horário
	
			P2OUT &= ~M5CK;        // OFF
			P7OUT |=  M5ACK;       // ON
			P2OUT |=  M5ENAB;      // ENABLE

		}else if (flagM5!=1){ // Parar o motor

			P2OUT &= ~M5CK;        // OFF
			P7OUT &= ~M5ACK;       // OFF
			P2OUT &= ~M5ENAB;      // DISABLE
		
//			__delay_cycles(500000);  // delay

			flagM5=1;	//	Avisa que chegou na posição
		} //Aguardando outros motores
		break;
		default: break;
	}
}