/*
 * File:   i2c.h
 * Author: xiang
 *
 * Created on November 14, 2025
 */

#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#define F_CPU 16000000UL
#define SCL_CLOCK  100000L

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);
uint8_t i2c_get_status(void);

#endif /* I2C_H */
