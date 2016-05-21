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

#define line 6
#define row  5

// Motor 1
#define	M1CK	BIT2
#define M1ACK 	BIT3
#define M1ENAB	BIT4

// Motor 2
#define	M2CK	BIT2
#define M2ACK 	BIT3
#define M2ENAB	BIT4

/* Parametros */
unsigned int SetPoint[line][row] = {{2000, 1800, 2400, 1800, 2000},
									{2100, 1900, 2500, 1900, 2100},
									{2200, 2000, 2600, 2000, 2200},
									{2300, 2100, 2700, 2100, 2300},
									{2400, 2200, 2800, 2200, 2400},
									{2500, 2300, 2900, 2300, 2500}};
/* SetPoint Rules

   SetPoint[line][row] = {{M1_Step1, M2_Step1, M3_Step1, M4_Step1, M5_Step1},
						  {M1_Step2, M2_Step2, M3_Step2, M4_Step2, M5_Step2},
				   		  {M1_StepN, M2_StepN, M3_StepN, M4_StepN, M5_StepN},}; */

unsigned int Gap = 40;			// Gap para a Histerese do Motor
unsigned int SetTarget;			// Valor para Ajuste do Erro de Posição
unsigned int SetPointMax;		// Valor Máximo
unsigned int SetPointMin;		// Valor Minimo

/* Analog Values */
unsigned int Results_A0[line];
unsigned int Results_A1[line];
unsigned int Results_A2[line];
unsigned int Results_A3[line];
unsigned int Results_A4[line];

unsigned int i = 0;

void Conf_timer(void);
void Conf_ADC(void);
void StateMachine(unsigned int SetPoint[line][row]);
void MotorCommand(unsigned int SetTarget, unsigned int Gap);

int main(void){

	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer

    // Configura Saídas
    P1DIR |=  BIT0;                         // Output P1.0
    P1DIR |=  M1CK + M1ACK + M1ENAB;        // Output P1.2, P1.3, P1.4
    P1OUT &= ~M1CK + M1ACK + M1ENAB;        // Forcar iniciar off

    P3DIR |=  M2CK + M2ACK + M2ENAB;        // Output P3.4, P3.5, P3.6
    P3OUT &= ~M2CK + M2ACK + M2ENAB;        // Forcar iniciar off

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

	Results_A0[i] = ADC12MEM0;      // Move A0 results, IFG is cleared
	Results_A1[i] = ADC12MEM1;      // Move A1 results, IFG is cleared
	Results_A2[i] = ADC12MEM2;      // Move A2 results, IFG is cleared
	Results_A3[i] = ADC12MEM3;      // Move A3 results, IFG is cleared
	Results_A4[i] = ADC12MEM4;	    // Move A4 results, IFG is cleared

    // Altera Posição do Vetor de Resultados
    i++;
    if (i == line) i = 0;

    StateMachine(SetPoint);


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


void StateMachine(unsigned int SetPoint[line][row]){


	for(i = 0; i < line; i++){

		SetTarget = SetPoint[i][1];
		MotorCommand(SetTarget, Gap);
		__delay_cycles(500000);

	}

}

void MotorCommand(unsigned int SetTarget, unsigned int Gap){

	SetPointMax = SetTarget + Gap;   // Seta Valor Maximo
	SetPointMin = SetTarget - Gap;   // Seta Valor Minimo

	// Girar Sentido Horário
	if( Results_A0[0] > SetPointMax){

		P1OUT |=  M1CK;        // P1.2 ON
		P1OUT &= ~M1ACK;       // P1.3 OFF

		P1OUT |=  M1ENAB;      // P1.4 ENABLE

		// Girar Sentido Antihorário
	}else if( Results_A0[0] < SetPointMin){

		P1OUT &= ~M1CK;        // P1.2 OFF
		P1OUT |=  M1ACK;       // P1.3 ON

		P1OUT |=  M1ENAB;      // P1.4 ENABLE

		// Manter Parado
	}else{

		P1OUT &= ~BIT2;        // P1.2 OFF
		P1OUT &= ~BIT3;        // P1.3 OFF

		P1OUT &= ~BIT4;        // P1.4 DISABLE

	}
}



