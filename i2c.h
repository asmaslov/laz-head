#ifndef __I2C_H__
#define __I2C_H__

#include "head_config.h"

#include <inttypes.h>

#define I2C_FREQUENCY_HZ  100000
#define I2C_MAX_ITER      10

void i2c_setup(void);
int i2c_readData(uint8_t i2cAddr, uint8_t subAddr, uint8_t* data, int len);
int i2c_writeData(uint8_t i2cAddr, uint8_t subAddr, uint8_t* data, int len);

#endif // __COMPORT_H__
