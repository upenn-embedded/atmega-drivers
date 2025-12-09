#include "i2c.h"
#include "uart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

// --- Global variables for timer ---
volatile uint16_t g_timer_ms_count = 0;
volatile uint8_t g_read_sensor_flag = 0;

// --- Values from the LSM6DSO Datasheet ---
#define LSM6DSO_DEVICE_ADDR 0x6B
#define LSM6DSO_WRITE_ADDR  ((LSM6DSO_DEVICE_ADDR << 1) | 0)
#define LSM6DSO_READ_ADDR   ((LSM6DSO_DEVICE_ADDR << 1) | 1)

// Register Addresses
#define LSM6DSO_WHO_AM_I_REG  0x0F
#define LSM6DSO_CTRL1_XL_REG  0x10
#define LSM6DSO_CTRL2_G_REG   0x11
#define LSM6DSO_OUTX_L_XL_REG 0x28

// Sensitivity for ±4g from datasheet
#define ACCEL_SENSITIVITY_4G 0.122f

// --- Function Prototypes ---
void lsm6dso_write_register(uint8_t reg_addr, uint8_t data);
uint8_t lsm6dso_read_register(uint8_t reg_addr);
void lsm6dso_read_accel(int16_t *ax, int16_t *ay, int16_t *az);
void system_init(void);

// -- Variable definitions --
float accel_x_g;
float accel_y_g;
float accel_z_g;

// --- Timer Interrupt Service Routine ---
// This ISR will be called every 1ms
ISR(TIMER0_COMPA_vect) {
    g_timer_ms_count++;
    if (g_timer_ms_count >= 100) {   // 100ms interval for readable output
        g_timer_ms_count = 0;
        g_read_sensor_flag = 1;   // Set the flag for the main loop
    }
}

int
main(void) {
    system_init();   // Initialize clock, UART, I2C, and Timer

    // Check the WHO_AM_I register to verify communication
    uint8_t who_am_i = lsm6dso_read_register(LSM6DSO_WHO_AM_I_REG);

    printf("WHO_AM_I value: 0x%02X\n", who_am_i);

    if (who_am_i == 0x6C) {
        printf("LSM6DSO communication successful!\n");
    } else {
        printf("Error: LSM6DSO not found or communication failed.\n");
        while (1)
            ;   // Halt on failure
    }

    lsm6dso_write_register(LSM6DSO_CTRL1_XL_REG, 0b01001000);   // Accel: 104Hz, ±4g
    lsm6dso_write_register(LSM6DSO_CTRL2_G_REG, 0b01000100);    // Gyro: 104Hz, ±500dps
    printf("LSM6DSO configured.\n");
    printf("Reading IMU data (X, Y, Z in g):\n");

    int16_t raw_ax, raw_ay, raw_az;

    while (1) {
        if (g_read_sensor_flag) {
            g_read_sensor_flag = 0;

            lsm6dso_read_accel(&raw_ax, &raw_ay, &raw_az);

            // Convert raw data to g's
            accel_x_g = (float) raw_ax * ACCEL_SENSITIVITY_4G / 1000.0f;
            accel_y_g = (float) raw_ay * ACCEL_SENSITIVITY_4G / 1000.0f;
            accel_z_g = (float) raw_az * ACCEL_SENSITIVITY_4G / 1000.0f;

            // Print X, Y, Z accelerometer data
            printf("X: %.3f  Y: %.3f  Z: %.3f\n", accel_x_g, accel_y_g, accel_z_g);
        }
    }

    return 0;   // Should not be reached
}

void
system_init(void) {
    cli();   // Disable global interrupts

    // Initialize peripherals
    uart_init(19200);
    i2c_init();

    // --- Timer0 Setup for 1ms interrupt ---
    // Set CTC Mode
    TCCR0A = (1 << WGM01);
    TCCR0B = 0;

    // Set OCR0A for 1ms tick at 16MHz clock with prescaler of 64
    // Formula: OCR0A = (F_CPU / Prescaler / Target_Freq) - 1
    // OCR0A = (16,000,000 / 64 / 1000) - 1 = 249
    OCR0A = 249;

    // Enable Timer0 Compare A interrupt
    TIMSK0 |= (1 << OCIE0A);

    // Set prescaler to 64 and start the timer
    TCCR0B |= (1 << CS01) | (1 << CS00);

    sei();   // Enable global interrupts
}

uint8_t
lsm6dso_read_register(uint8_t reg_addr) {
    uint8_t data;
    i2c_start();
    i2c_write(LSM6DSO_WRITE_ADDR);
    i2c_write(reg_addr);
    i2c_start();   // Repeated start
    i2c_write(LSM6DSO_READ_ADDR);
    data = i2c_read_nack();
    i2c_stop();
    return data;
}

void
lsm6dso_write_register(uint8_t reg_addr, uint8_t data) {
    i2c_start();
    i2c_write(LSM6DSO_WRITE_ADDR);   // imu address with write bit
    i2c_write(reg_addr);             // register to write to
    i2c_write(data);                 // data to write
    i2c_stop();
}

void
lsm6dso_read_accel(int16_t *ax, int16_t *ay, int16_t *az) {
    i2c_start();
    i2c_write(LSM6DSO_WRITE_ADDR);      // imu address with write bit set
    i2c_write(LSM6DSO_OUTX_L_XL_REG);   // register to read from
    i2c_start();                        // Repeated start
    i2c_write(LSM6DSO_READ_ADDR);       // imu address with read bit set

    uint8_t ax_l = i2c_read_ack();   // read accelerations, low and high bits
    uint8_t ax_h = i2c_read_ack();
    uint8_t ay_l = i2c_read_ack();
    uint8_t ay_h = i2c_read_ack();
    uint8_t az_l = i2c_read_ack();
    uint8_t az_h = i2c_read_nack();
    i2c_stop();

    // Compile low and high bits into acceleration data
    *ax = (int16_t) ((ax_h << 8) | ax_l);
    *ay = (int16_t) ((ay_h << 8) | ay_l);
    *az = (int16_t) ((az_h << 8) | az_l);
}
