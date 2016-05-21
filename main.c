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
#define line 5
#define row 6

#define	M1CK	BIT2
#define M1ACK 	BIT3
#define M1ENAB	BIT4

#define	M2CK	BIT2
#define M2ACK 	BIT3
#define M2ENAB	BIT4

unsigned int gap[line] = {40, 40, 40, 40, 40};
unsigned int SetPoint[line][row] = {{2000, 1800, 2400, 1800, 2000},
									{2100, 1900, 2500, 1900, 2100},
									{2200, 2000, 2600, 2000, 2200},
									{2300, 2100, 2700, 2100, 2300},
									{2400, 2200, 2800, 2200, 2400},
									{2500, 2300, 2900, 2300, 2500},};
unsigned int SetPointMax[line];
unsigned int SetPointMin[line];

unsigned int Results_A0[line];
unsigned int Results_A1[line];
unsigned int Results_A2[line];
unsigned int Results_A3[line];
unsigned int Results_A4[line];

unsigned int i = 0;

void Conf_timer(void);
void Conf_ADC(void);

int main(void){

	WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer

    // Configura Saídas
    P1DIR |=  BIT0;                         // Output P1.0
    P1DIR |=  M1CK + M1ACK + M1ENAB;        // Output P1.2, P1.3, P1.4
    P1OUT &= ~M1CK + M1ACK + M1ENAB;        // Forcar iniciar off

    P3DIR |=  M2CK + M2ACK + M2ENAB;           // Output P3.4, P3.5, P3.6
    P3OUT &= ~M2CK + M2ACK + M2ENAB;           // Forcar iniciar off

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
	Results_A4[i] = ADC12MEM4;	// Move A4 results, IFG is cleared

        /**********************************************************************/

        SetPointMax[0] = SetPoint[0] + gap[0];   // Seta Valor Maximo
        SetPointMin[0] = SetPoint[0] - gap[0];   // Seta Valor Minimo

        // Girar Sentido Horário
        if( Results_A0[0] > SetPointMax[0] ){

            P1OUT |=  M1CK;        // P1.2 ON
            P1OUT &= ~M1ACK;        // P1.3 OFF

            P1OUT |=  M1ENAB;        // P1.4 ENABLE

        // Girar Sentido Antihorário
        }else if( Results_A0[0] < SetPointMin[0] ){

            P1OUT &= ~M1CK;        // P1.2 OFF
            P1OUT |=  M1ACK;        // P1.3 ON

            P1OUT |=  M1ENAB;        // P1.4 ENABLE

        // Manter Parado
        }else{

            P1OUT &= ~BIT2;        // P1.2 OFF
            P1OUT &= ~BIT3;        // P1.3 OFF

            P1OUT &= ~BIT4;        // P1.4 DISABLE

        /**********************************************************************/

            switch(SetPoint[0]){
            case  2100:
              __delay_cycles(500000);
              SetPoint[0] = 2800;
            break;
            case  2800:
              __delay_cycles(500000);
              SetPoint[0] = 3100;
            break;
            case  3100:
              __delay_cycles(500000);
              SetPoint[0] = 2500;
            break;
            default: break;
            }
        }

        /******************************************************************/


        SetPointMax[1] = SetPoint[1] + gap[1];   // Seta Valor Maximo
        SetPointMin[1] = SetPoint[1] - gap[1];   // Seta Valor Minimo

        // Girar Sentido Horário
        if( Results_A1[0] > SetPointMax[1] ){

            P3OUT |=  M2CK;        // P1.2 ON
            P3OUT &= ~M2ACK;        // P1.3 OFF

            P3OUT |=  M2ENAB;        // P1.4 ENABLE

        // Girar Sentido Antihorário
        }else if( Results_A1[0] < SetPointMin[1] ){

            P3OUT &= ~M2ENAB;        // P1.2 OFF
            P3OUT |=  M2ENAB;        // P1.3 ON

            P3OUT |=  M2ENAB;        // P1.4 ENABLE

        // Manter Parado
        }else{

            P3OUT &= ~BIT5;        // P1.2 OFF
            P3OUT &= ~BIT6;        // P1.3 OFF

            P3OUT &= ~BIT4;        // P1.4 DISABLE

        /**********************************************************************/

            switch(SetPoint[1]){
            case  2100:
              __delay_cycles(500000);
              SetPoint[1] = 1800;
            break;
            case  1800:
              __delay_cycles(500000);
              SetPoint[1] = 2400;
            break;
            case  2400:
              __delay_cycles(500000);
              SetPoint[1] = 1900;
            break;
            default: break;
            }
        }



        // Altera Posição do Vetor de Resultados
        i++;

        if (i == Tam){
          i = 0;
        }
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

	ADC12IE = 0x08;         // Enable ADC12IFG.3 - ?

}


void StateMachine(unsigned int SetPoint){


    switch(SetPoint[0]){
		case  2100:
		  __delay_cycles(500000);
		  SetPoint[0] = 2800;
		break;
		case  2800:
		  __delay_cycles(500000);
		  SetPoint[0] = 3100;
		break;
		case  3100:
		  __delay_cycles(500000);
		  SetPoint[0] = 2500;
		break;
		default: break;
	}

}



