#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
  {
    DDRB = (1 << DDB0) | (1 << DDB1) | (1 << DDB2);
    DDRD = 0x00;
    PORTB = 0x00;

    while(1){
    PORTB |= (1 << PB1);
    _delay_ms(1000);
    
    PORTB &= ~(1 << PB1);
    _delay_ms(1000);
    

    if (PIND & (1 << PD2))
        {
        PORTB |= (1 << PB0);
        //_delay_ms(5000);
        }
    else
        {
            PORTB &= ~(1 << PB0);
        }
    }
}