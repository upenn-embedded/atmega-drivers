#include <stdio.h>
#include "uart.h"
#include <avr/io.h>
#include <stdarg.h>
#include <string.h>
// --- Function Implementations ---

// Function that printf will call to send a character
int uart_putchar(char c, FILE *stream) {
    // Add a carriage return before every newline character
    if (c == '\n') {
        uart_putchar('\r', stream);
    }
    // Wait for the transmit buffer to be empty
    while (!(UCSR0A & (1 << UDRE0)));
    // Put the character into the buffer to send it
    UDR0 = c;
    return 0;
}

// Function that scanf will call to receive a character
static int uart_getchar(FILE *stream) {
    // Wait for data to be received
    while (!(UCSR0A & (1 << RXC0)));
    // Get and return received data from buffer
    return UDR0;
}

// Create FILE streams for stdout (writing) and stdin (reading)
// This links our custom functions to the standard I/O streams.
static FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
static FILE uart_input  = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

// --- Public Functions ---

void uart_init(unsigned long baud) {
    // Calculate the UBRR value from F_CPU and baud rate
    // F_CPU must be defined in your project settings (e.g., platformio.ini)
    unsigned int ubrr = (F_CPU / 16 / baud) - 1;

    // Set baud rate registers
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    // Enable both transmitter and receiver
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);

    // Set frame format: 8 data bits, 1 stop bit (most common)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    // --- This is the correct way to redirect stdio ---
    // Assign the global stdout and stdin pointers to our custom streams
    stdout = &uart_output;
    stdin  = &uart_input;
}

int uart_send(char data, FILE* stream)
{
    // Wait for empty transmit buffer
    while(!(UCSR0A & (1<<UDRE0)));
    // Put data into buffer and send data
    UDR0 = data;
    return 0;
}

int uart_receive(FILE* stream)
{
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void determine_line_ending() {
    char c;
    printf("Press Enter to detect the line ending style...\n");

    while(1)
    {
        c = uart_receive(NULL);
        if (c == '\r') {
            printf("\\r (CR) detected.\n");
        } else if (c == '\n') {
            printf("\\n (LF) detected.\n");
        } else {
            printf("Unknown line ending.\n");
        }
    }
}

// Only integer (%d), char (%c), and string (%s) format specifiers have been implemented
#if !defined(CR) && !defined(LF) && !defined(CRLF)
#error "No line termination defined! #define one out of CR, LF, or CRLF"
#else
#ifdef MAX_STRING_LENGTH
void uart_scanf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const char* p = format;
    char buffer[MAX_STRING_LENGTH];
    int i = 0, num = 0;
    char c;

    while (*p) {
        if (*p == '%') 
        {
            p++;
            switch (*p) 
            {
                case 'd':
                {
                    num = 0;
                    while(1) {
                        c = uart_receive(NULL);
                        if(c >= '0' && c <= '9') 
                        {
                            num = num * 10 + (c - '0');
                        }
                        #if defined(CR) || defined(CRLF)
                        else if(c == '\r')
                        {
                            #ifdef CRLF
                            uart_receive(NULL);
                            #endif
                            break;
                        }
                        #endif
                        #ifdef LF
                        else if(c == '\n')
                        {
                            break;
                        }
                        #endif
                    }
                    int *int_ptr = va_arg(args, int*);
                    *int_ptr = num;
                    break;
                }
                case 's':
                {
                    i = 0;
                    while(1) 
                    {
                        c = uart_receive(NULL);

                        #if defined(CR) || defined(CRLF) 
                        if(c == '\r')
                        {
                            #ifdef CRLF
                            uart_receive(NULL);
                            #endif
                            buffer[i] = '\0';
                            break;
                        }
                        #endif
                        #ifdef LF
                        if(c == '\n')
                        {
                            buffer[i] = '\0';
                            break;
                        }
                        #endif

                        if(i < MAX_STRING_LENGTH - 1 && c != '\r' && c != '\n') 
                        {
                            buffer[i++] = c;
                        }
                    }
                    buffer[i] = '\0';
                    char *str_ptr = va_arg(args, char*);
                    strcpy(str_ptr, buffer);
                    break;
                }
                case 'c':
                {
                    while(1)
                    {
                        c = uart_receive(NULL);
                        #if defined(CR) || defined(CRLF)
                        if(c == '\r') 
                        {
                            #ifdef CRLF
                            c = uart_receive(NULL);
                            #endif
                            break;
                        }
                        #endif
                        #ifdef LF
                        if(c == '\n') 
                        {
                            break;
                        }
                        #endif
                        char *char_ptr = va_arg(args, char*);
                        *char_ptr = c;
                    }
                }
            }
        }
        p++;
    }
    va_end(args);
}
#else
#error "MAX_STRING_LENGTH undefined"
#endif
#endif
