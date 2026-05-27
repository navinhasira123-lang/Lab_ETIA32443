#include <Arduino.h>
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// ---------------- LED Pins ----------------
// Road LEDs
#define ROAD_RED      PB0
#define ROAD_YELLOW   PB1
#define ROAD_GREEN    PB2

// Pedestrian LEDs
#define PED_RED       PB3
#define PED_GREEN     PB4

// ---------------- Button Pins ----------------
// INT0 -> PD2
// INT1 -> PD3
// PCINT -> PC0

volatile uint8_t emergency_flag = 0;
volatile uint8_t pedestrian_flag = 0;
volatile uint8_t maintenance_flag = 0;

// ---------------- Functions ----------------

void all_leds_off()
{
    PORTB &= ~((1 << ROAD_RED) |
               (1 << ROAD_YELLOW) |
               (1 << ROAD_GREEN) |
               (1 << PED_RED) |
               (1 << PED_GREEN));
}

void normal_red_state()
{
    PORTB |= (1 << ROAD_RED);
    PORTB &= ~((1 << ROAD_GREEN) | (1 << ROAD_YELLOW));

    PORTB |= (1 << PED_GREEN);
    PORTB &= ~(1 << PED_RED);
}

void normal_green_state()
{
    PORTB |= (1 << ROAD_GREEN);
    PORTB &= ~((1 << ROAD_RED) | (1 << ROAD_YELLOW));

    PORTB |= (1 << PED_RED);
    PORTB &= ~(1 << PED_GREEN);
}

void emergency_mode()
{
    emergency_flag = 0;

    // Pedestrian RED
    PORTB |= (1 << PED_RED);
    PORTB &= ~(1 << PED_GREEN);

    // Road GREEN
    PORTB |= (1 << ROAD_GREEN);
    PORTB &= ~((1 << ROAD_RED) | (1 << ROAD_YELLOW));

    for(int i = 0; i < 10; i++)
    {
        _delay_ms(1000);
    }
}

void pedestrian_mode()
{
    pedestrian_flag = 0;

    // Yellow ON with current signal
    PORTB |= (1 << ROAD_YELLOW);

    for(int i = 0; i < 5; i++)
    {
        _delay_ms(1000);
    }

    // Road RED
    PORTB |= (1 << ROAD_RED);
    PORTB &= ~((1 << ROAD_GREEN) | (1 << ROAD_YELLOW));

    // Pedestrian GREEN
    PORTB |= (1 << PED_GREEN);
    PORTB &= ~(1 << PED_RED);

    for(int i = 0; i < 10; i++)
    {
        _delay_ms(1000);
    }
}

void maintenance_mode()
{
    while(maintenance_flag)
    {
        all_leds_off();

        PORTB |= (1 << ROAD_YELLOW);
        _delay_ms(500);

        PORTB &= ~(1 << ROAD_YELLOW);
        _delay_ms(500);

        // Exit maintenance mode if button pressed again
        if(!(PINC & (1 << PC0)))
        {
            _delay_ms(200);

            if(!(PINC & (1 << PC0)))
            {
                maintenance_flag = 0;
            }
        }
    }
}

// ---------------- Interrupts ----------------

// INT0 -> Emergency
ISR(INT0_vect)
{
    emergency_flag = 1;
}

// INT1 -> Pedestrian
ISR(INT1_vect)
{
    pedestrian_flag = 1;
}

// Pin Change Interrupt -> Maintenance
ISR(PCINT1_vect)
{
    maintenance_flag = 1;
}

// ---------------- Main ----------------

int main(void)
{
    // LED pins as OUTPUT
    DDRB |= (1 << ROAD_RED) |
            (1 << ROAD_YELLOW) |
            (1 << ROAD_GREEN) |
            (1 << PED_RED) |
            (1 << PED_GREEN);

    // Button pins as INPUT
    DDRD &= ~((1 << PD2) | (1 << PD3));
    DDRC &= ~(1 << PC0);

    // Enable pull-up resistors
    PORTD |= (1 << PD2) | (1 << PD3);
    PORTC |= (1 << PC0);

    // -------- Interrupt Configuration --------

    // INT0 falling edge
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);

    // INT1 falling edge
    EICRA |= (1 << ISC11);
    EICRA &= ~(1 << ISC10);

    // Enable INT0 and INT1
    EIMSK |= (1 << INT0) | (1 << INT1);

    // Enable Pin Change Interrupt for PC0
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT8);

    // Global interrupt enable
    sei();

    while(1)
    {
        // Highest priority
        if(maintenance_flag)
        {
            maintenance_mode();
        }

        if(emergency_flag)
        {
            emergency_mode();
        }

        if(pedestrian_flag)
        {
            pedestrian_mode();
        }

        // Normal operation
        normal_red_state();

        for(int i = 0; i < 5; i++)
        {
            _delay_ms(1000);

            if(emergency_flag || pedestrian_flag || maintenance_flag)
                break;
        }

        if(emergency_flag || pedestrian_flag || maintenance_flag)
            continue;

        normal_green_state();

        for(int i = 0; i < 5; i++)
        {
            _delay_ms(1000);

            if(emergency_flag || pedestrian_flag || maintenance_flag)
                break;
        }
    }
}