/*
 * proj.c
 *
 *  Created on: Sep 15, 2022
 *      Author: Basma Walid
 */

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>


unsigned char flag=0;

unsigned char digits[6];

ISR(TIMER1_COMPA_vect)
{
	SREG |= (1<<7); /* Enable the I-bit for Interrupt Nesting */

	/*When Timer reaches compare value , Set the flag variable  */
	flag=1;
}
ISR(INT0_vect)
{
	SREG |= (1<<7); /* Enable the I-bit for Interrupt Nesting */
	unsigned char i=0;
	for(i=0 ; i<6 ; i++)
		digits[i]=0; /* RESET */
}

ISR(INT1_vect)
{
	SREG |= (1<<7); /* Enable the I-bit for Interrupt Nesting */
	TCCR1B &= ~ (1<<CS10) ; /* Disabling the clock of the Timer --> Stop the count*/

}

ISR(INT2_vect)
{
	SREG |= (1<<7); /* Enable the I-bit for Interrupt Nesting */
	TCCR1B |= (1<<CS10); /* Clock and Mode selection bits For timer --> Enable the Timer */
}

void TIMER1_Init()
{
	TCNT1 = 0; /* Initial Count value =0 */
	OCR1A=1000; /* Compare Value = 1000 */
	TIMSK=(1<<OCIE1A); /* Enabling the output compare Enable */
	TCCR1A=(1<<FOC1A); /* Non-PWM Mode */
	TCCR1B=(1<<WGM12) | (1<<CS10) | (1<<CS12); /* Clock and Mode selection bits ( N=1024 ) */
}


void INTO_Init(void)
{
	MCUCR = (1<<ISC01); /* Button works with Falling Edge */
	GICR |= (1<<INT0); /* Enabling the module interrupt enable */
}


void INT1_Init(void)
{
	MCUCR = (1<<ISC11) | (1<<ISC10); /* Rising Edge */
	GICR |= (1<<INT1); /* Enabling the module interrupt enable */
}

void INT2_Init(void)
{
	MCUCSR &= ~(1<<ISC2); /* Falling Edge */
	GICR |= (1<<INT2); /* Enabling the module interrupt enable */
}



void update_stop_watch_digits()
{
	/* when timer 1 ISR is completed , Increment the stop watch */

	digits[0]++;
	/* Incrementing the seconds counter */


	if(digits[0]==10)
	{ /* First Seconds counter reaches 10 : Clear it and Increment the Second Seconds counter */
		digits[0]=0;
		digits[1]++;

		/* We reached a minute */
		if(digits[1]==6 && digits[0]==0)
		{	/* Increment the First Minute counter and Clear the seconds counter*/
			digits[1]=0;
			digits[2]++;
		}
	}

	/* When we reach 10 Minutes */
	if(digits[2]==10)
	{	/* Clear First Minute counter and increment the second Minute Counter*/
		digits[2]=0;
		digits[3]++;

		/* We reached an hour */
		if(digits[3]==6 && digits[2]==0)
		{	/* Increment the First Hour Counter and Clear the Minutes Counter */
			digits[3]=0;
			digits[4]++;
		}
	}

	/* We reach 10 Hours */
	if(digits[4]==10)
	{	/* Clear The first Hour counter and increment the second hour counter */
		digits[4]=0;
		digits[5]++;
		if(digits[5]==10 && digits[4]==0)
		{
				/* We reach Maximum Value for counting(99hours:59minutes:59seconds) */
				/* Reseting The stop_watch */
				for(int i=0;i<6;i++)
				{
					digits[i]=0;
				}
				TCNT1=0;
		}
	}

}
void SEVEN_SEGMENT_DISPLAY()
{
	unsigned char i;
	for(i=0 ; i<6 ; i++)
	{
		PORTA=1<<i;

		PORTC=digits[i];

		_delay_ms(3);

	}
	/* Enable the 6 Enables quickly so that we can see all of the 6-digits at once */
}

int main(void)
{
	DDRC |= 0x0F ; /* PC0,PC1,PC2 AND PC3 output pins */

	DDRA |=0x3F; /* PA0:5 Output pins */
	/* Enables for the 6 7-Segments */

	DDRB &= ~(1<<2); /* Button Input(RESUME BUTTON) */
	PORTB |= (1<<2); /* Activating the pull up resistor */
	/* Button_1 Logic : 0-->pushed , 1-->Not pushed */

	DDRD &= ~(1<<2); /* Button Input(RESET BUTTON) */
	PORTD |= (1<<2); /* Activating the pull up resistor */
	/* Button_2 Logic : 0-->pushed , 1-->Not pushed */

	DDRD &= ~(1<<3); /*  Button Input(PAUSE BUTTON)  */
	/* Button_3 Logic : 1-->pushed , 0-->Not pushed */

	PORTA |= (0x3F); /* PA0:5 are 1's initially */
	PORTC &= ~(0xF0); /* PC0,PC1,PC2 AND PC3 output 0 initially */


	SREG |= (1<<7); /* Enable the I-bit */

	TIMER1_Init(); /* Calling The Timer-setup Function */


	INTO_Init(); /* Calling The INT0-setup Function */
	INT1_Init(); /* Calling The INT1-setup Function */
	INT2_Init(); /* Calling The INT2-setup Function */


	while(1)
	{

		SEVEN_SEGMENT_DISPLAY();
		if(flag==1)
			{
			update_stop_watch_digits();
			flag=0;
			}

	}

}

