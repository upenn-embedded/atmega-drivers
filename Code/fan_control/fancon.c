// -------------------------------------
// Fan Control with UART Input
// Receives commands from ESP32 and controls fan PWM
// -------------------------------------
// Commands:
//   'w' - Low speed (25% duty cycle)
//   's' - Medium speed (50% duty cycle)
//   'W' - High speed (80% duty cycle)
//   'x' - Stop fan (0% duty cycle)
// -------------------------------------

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

#define F_CPU 16000000UL

// PWM settings for 25kHz fan control (typical PC fan frequency)
// TOP = (16MHz / (1 * 25kHz)) - 1 = 639
// Using prescaler 1 for fine control
#define PWM_TOP 639

// Duty cycle values (percentage of PWM_TOP)
#define DUTY_OFF 0
#define DUTY_LOW (PWM_TOP * 25 / 100)    // 25%
#define DUTY_MEDIUM (PWM_TOP * 50 / 100) // 50%
#define DUTY_HIGH (PWM_TOP * 80 / 100)   // 80%

void pwm_init(void)
{
    // Set PD5 (OC0B) as output for PWM
    DDRD |= (1 << DDD5);

    // Timer0, prescale of 1
    TCCR0B |= (1 << CS00);

    // Timer0, Fast PWM mode with TOP = OCR0A (Mode 7)
    TCCR0A |= (1 << WGM00) | (1 << WGM01);
    TCCR0B |= (1 << WGM02);

    // Set TOP value for ~25kHz (or adjust for your fan)
    // For 8-bit timer, max is 255, so we use a different approach
    // Using Mode 3 (Fast PWM, TOP=0xFF) with prescaler 1: f = 16MHz/256 = 62.5kHz
    // Or Mode 7 with OCR0A as TOP
    OCR0A = 255; // TOP value (adjust for desired frequency)
    OCR0B = 0;   // Start with fan off

    // Non-inverting mode on OC0B (Clear on Compare Match)
    TCCR0A |= (1 << COM0B1);
    TCCR0A &= ~(1 << COM0B0);
}

void set_fan_speed(uint8_t duty)
{
    // duty is 0-255 for 0-100%
    OCR0B = duty;
}

int main(void)
{
    // Initialize UART for receiving commands from ESP32
    uart_init(9600);

    // Initialize PWM for fan control
    pwm_init();

    // Enable global interrupts (if needed)
    sei();

    printf("Fan Control Ready\n");
    printf("Commands: w=low, s=medium, W=high, x=stop\n");

    while (1)
    {
        // Check if data is available on UART
        if (UCSR0A & (1 << RXC0))
        {
            char cmd = UDR0; // Read received character

            switch (cmd)
            {
            case 'w': // Low speed
                set_fan_speed(DUTY_LOW);
                printf("Fan: LOW (25%%)\n");
                break;

            case 's': // Medium speed
                set_fan_speed(DUTY_MEDIUM);
                printf("Fan: MEDIUM (50%%)\n");
                break;

            case 'W': // High speed
                set_fan_speed(DUTY_HIGH);
                printf("Fan: HIGH (80%%)\n");
                break;

            case 'x': // Stop
            case 'X':
                set_fan_speed(DUTY_OFF);
                printf("Fan: STOPPED\n");
                break;

            default:
                // Echo back unknown command
                printf("Unknown: %c\n", cmd);
                break;
            }
        }
    }

    return 0;
}