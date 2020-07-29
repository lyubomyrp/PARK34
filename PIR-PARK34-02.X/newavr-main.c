/*
 * File:   newavr-main.c
 * Author: lyubomyr.pasechko
 *
 * Created on May 26, 2020, 12:53 PM
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/portpins.h>
#include <stdbool.h>
#include <stdio.h>


#define ADCRESOLUTION   19  // ADC 19mv per 1 bit
#define PIRMID          85  // ADCPIR no signal level. Offset voltage 1.1V
#define PIRHIGH         98  // ADC High level TRIGGER
#define PIRLOW          72  // ADC LOW level Trigger


uint16_t    Scount;         // 1 sec counter
uint16_t     Mcount;         // 1 min counter
uint8_t     S3count;        // 3 sec counter, LED counter


ISR(TIM0_OVF_vect)
{
    /*1ms timer*/
    TCNT0L = 0XD2;          // timer compare register reset to 1 ms period
    TCNT0H = 0XFF;          // timer compare register reset to 1 ms period

    if(Scount>0) Scount--;  // 1 sec counter
    else {
        
        Scount = 1000;
        if(S3count>0)                  // 3 sec counter (led on counter)
        {
            S3count--; 
            PORTB &= ~(1 << PB1);       // led 0n
            
        }
        else{
            PORTB|=(1 << PB1);          // led off
        }
        
            if(Mcount>0)              // 20 min counter
            {
                Mcount--;             
                PORTB|=(1 << PB2);      // dimmer output on
            }
            else{
                PORTB &= ~(1 << PB2);   // dimmer output off
            }
    }
    
    if((ADCL>PIRHIGH) || (ADCL<PIRLOW)) //if movement detected
    {
        Mcount = 1200;                  // Reset 20 minutes timer
        S3count = 1;                    // turn LED on for 3 sec
    }
    else
    {
      ;   
    }
            
    wdt_reset();     //Reset Watchdog
};

int main(void) {
    
    /*Clock setup*/ 
 
   
    CCP = 0XD8;             // unlock
    CLKMSR = 0b00000000;    // Calibrated Internal 8 MHzOscillator
    CCP = 0xD8;             // wite access signature to Configuration Change Protection Register
    //CLKPSR = 0;           // Prescaler 1
    CLKPSR |= (0<<CLKPS0) | (0<<CLKPS1) | (1<<CLKPS2) | (1<<CLKPS3); // Prescaler 16  (500Khz))
    
    /*Pin Configuration*/
    DDRB = 0b0110;
    
    /*ADC*/
    PRR |= (0<< PRADC);                             //ADC power on
    ADMUX  |= (0<< MUX1) | (0<< MUX0);              // ADC0 --> PB0
    ADCSRA |= (1<<ADATE);                           // auto triggering mode
    ADCSRB |= (0<<ADTS0) |(0<<ADTS1) |(0<<ADTS2);   //ADC in free running mode
    ADCSRA |= (1<< ADEN);                           //ADC enable
    ADCSRA |= (1<<ADSC);                            // ADC start conversation

    /*timer setup*/
    TCCR0A |= (0 <<WGM00) | (0 <<WGM01);
    TCCR0B |= (0 <<WGM02) | (0 <<WGM03);
    TCCR0B |= (0 << CS02) | (0 << CS01) |( 1 <<CS00);  // Prescaler 1 internal clock select 
    TIMSK0 |= (1 <<TOIE0);                             // overflow interrupt is enabled    
    
    /*WD setup*/
    RSTFLR |= 1<< WDRF;
    CCP = 0XD8;  
    WDTCSR |= (1<< WDE) | (1<< WDP3) | (0<< WDP2)| (0<< WDP1)| (1<< WDP0) ;      // Enable watchdog end prescaler
    
    /*Enable all interrupts*/
    sei();  
    
    while (1) 
    {

    }
}
