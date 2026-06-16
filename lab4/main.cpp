#include <avr/io.h>
#include <util/delay.h>

#define INITIAL_CAPACITY 10

uint8_t vehicles_entered = 0;
uint8_t available_slots = INITIAL_CAPACITY;

uint8_t vehicleDetected = 0;

long getDistance()
{
    PORTB &= ~(1 << PB1);      // Trigger LOW
    _delay_us(2);

    PORTB |= (1 << PB1);       // Trigger HIGH
    _delay_us(10);

    PORTB &= ~(1 << PB1);      // Trigger LOW

    while (!(PINB & (1 << PB2))); // Wait for echo HIGH

    uint32_t count = 0;

    while (PINB & (1 << PB2))
    {
        _delay_us(1);
        count++;
    }

    return count / 58;   // Convert to cm
}

void updateLEDs()
{
    PORTD &= ~((1 << PD2) | (1 << PD3) | (1 << PD4));

    if (available_slots > INITIAL_CAPACITY / 2)
    {
        PORTD |= (1 << PD4);   // Green
    }
    else if (available_slots > 0)
    {
        PORTD |= (1 << PD3);   // Yellow
    }
    else
    {
        PORTD |= (1 << PD2);   // Red
    }
}

int main(void)
{
    // LEDs OUTPUT
    DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4);

    // Trigger OUTPUT
    DDRB |= (1 << PB1);

    // Echo INPUT
    DDRB &= ~(1 << PB2);

    // Button INPUT_PULLUP
    DDRD &= ~(1 << PD7);
    PORTD |= (1 << PD7);

    updateLEDs();

    while (1)
    {
        long distance = getDistance();

        uint8_t currentState = (distance < 10);

        // State-change detection
        if (currentState && !vehicleDetected)
        {
            if (vehicles_entered < INITIAL_CAPACITY)
            {
                vehicles_entered++;
                available_slots = INITIAL_CAPACITY - vehicles_entered;
                updateLEDs();
            }

            vehicleDetected = 1;
        }

        if (!currentState)
        {
            vehicleDetected = 0;
        }

        // Reset button pressed
        if (!(PIND & (1 << PD7)))
        {
            vehicles_entered = 0;
            available_slots = INITIAL_CAPACITY;

            updateLEDs();

            _delay_ms(300);
        }

        _delay_ms(100);
    }
}