/*
 * File:   i2c.c
 * Author: xiang
 *
 * Created on November 14, 2025, 11:39 AM
 */

#include <avr/io.h>
#include "i2c.h"

void i2c_init(void){
    // Set TWI register to 0
    TWSR0 = 0; //FIX: Was TWSR
    //sets TWI Bit Rate Register to SCL_CLOCK frequency
    TWBR0 = ((F_CPU/SCL_CLOCK)-16)/2; //FIX: Was TWBR
    // Enable TWI
    TWCR0 = (1<<TWEN); //FIX: Was TWCR
}

void i2c_start(void){
    //clears TWINT
    //sets TWI start condition
    //ensures TWI is enabled
    TWCR0 = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); //FIX: Was TWCR
    //busy wait loop until hardware starts
    while (!(TWCR0 & (1<<TWINT))); //FIX: Was TWCR
}

void i2c_stop(void){
    //TWSTO sets TWI stop condition
    TWCR0 = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN); //FIX: Was TWCR
}

void i2c_write(uint8_t data){
    TWDR0 = data; //FIX: Was TWDR
    TWCR0 = (1<<TWINT)|(1<<TWEN); //FIX: Was TWCR
    while (!(TWCR0 & (1<<TWINT))); //FIX: Was TWCR
}

uint8_t i2c_read_ack(void){
    TWCR0 = (1<<TWINT)|(1<<TWEN)|(1<<TWEA); //FIX: Was TWCR
    while (!(TWCR0 & (1<<TWINT))); //FIX: Was TWCR
    return TWDR0; //FIX: Was TWDR
}

uint8_t i2c_read_nack(void){
    TWCR0 = (1<<TWINT)|(1<<TWEN); //FIX: Was TWCR
    while (!(TWCR0 & (1<<TWINT))); //FIX: Was TWCR
    return TWDR0; //FIX: Was TWDR
}

uint8_t i2c_get_status(void){
    uint8_t status;
    //mask status
    status = TWSR0 & 0xF8; //FIX: Was TWSR
    return status;
}