#include "lsm303.h"
#include "i2c.h"
#include "debug.h"

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <inttypes.h>
#include <math.h>

static const uint32_t prescale0[6] = {0, 1, 8, 64, 256, 1024};
  
static uint32_t timerStep = 0;
static uint8_t timerConfig = 0;

ISR(TIMER0_COMP_vect)
{
  wdt_reset();
  timerStep++;
  if (timerStep > 100)
  {
    timerStep = 0;
    lsm303_get(&lsm303_anglesReal);
  }
}

static bool lsm303a_writeReg(uint8_t regAddr, uint8_t data)
{
  if (i2c_writeData(LSM303A_I2C_ADDR, regAddr, &data, 1) != 1)
  {
    return false;
  }
  return true;
}

static bool lsm303a_readReg(uint8_t regAddr, uint8_t *dest, uint16_t count)
{
  if (count > 1)
  {
    regAddr |= LSM303M_I2C_BURST_ADDR_BIT;
  }
  if (i2c_readData(LSM303A_I2C_ADDR, regAddr, dest, count) != count)
  {
    return false;
  }
  return true;
}

static bool lsm303m_writeReg(uint8_t regAddr, uint8_t data)
{
  if (i2c_writeData(LSM303M_I2C_ADDR, regAddr, &data, 1) != 1)
  {
    return false;
  }
  return true;
}

static bool lsm303m_readReg(uint8_t regAddr, uint8_t *dest, uint16_t count)
{
  if (count > 1)
  {
    regAddr |= LSM303M_I2C_BURST_ADDR_BIT;
  }
  if (i2c_readData(LSM303M_I2C_ADDR, regAddr, dest, count) != count)
  {
    return false;
  }
  return true;
}

bool lsm303a_init(void)
{
  uint8_t reg;
  
  lsm303_accelReal.x = 0;
  lsm303_accelReal.y = 0;
  lsm303_accelReal.z = 0;
  if (!lsm303a_writeReg(LSM303A_CTRL_REG1,
                        LSM303A_CTRL_REG1_ENABLE_X |
                        LSM303A_CTRL_REG1_ENABLE_Y |
                        LSM303A_CTRL_REG1_ENABLE_Z |
                        LSM303A_CTRL_REG1_DATARATE_100_HZ))
  {
    return false;
  }
  if (!lsm303a_readReg(LSM303A_CTRL_REG1, &reg, 1))
  {
    return false;
  }
  if (!lsm303a_writeReg(LSM303A_CTRL_REG4,
                        LSM303A_CTRL_REG4_FULLSCALE_2G))
  {
    return false;
  }
  if (!lsm303a_readReg(LSM303A_CTRL_REG1, &reg, 1))
  {
    return false;
  }
  return true;
}

bool lsm303m_init(void)
{
  uint8_t reg;
  
  lsm303_magnetReal.x = 0;
  lsm303_magnetReal.y = 0;
  lsm303_magnetReal.z = 0;
  if (!lsm303m_writeReg(LSM303M_CRB_REG,
                        LSM303M_CRB_REG_FULLSCALE_1_3GA))
  {
    return false;
  }
  if (!lsm303a_readReg(LSM303M_CRB_REG, &reg, 1))
  {
    return false;    
  }
  if (!lsm303m_writeReg(LSM303M_MR_REG,
                        LSM303M_MR_REG_MODE_CONTINIOUS))
  {
    return false;
  }
  if (!lsm303a_readReg(LSM303M_MR_REG, &reg, 1))
  {
    return false;    
  }
  return true;
}

bool lsm303a_read(LSM303_VALUES* accel)
{
  uint8_t buffer[6], ctrl[2], stat;
  int16_t rawX, rawY, rawZ;
  float sens;
  
  if (lsm303a_readReg(LSM303A_OUT_X_L, buffer, 6) &&
      lsm303a_readReg(LSM303A_CTRL_REG4, ctrl, 2) &&
      lsm303a_readReg(LSM303A_STATUS_REG, &stat, 1))
  {
    if(ctrl[1] & LSM303A_CTRL_REG5_FIFO_ENABLE)
    {
    sens = LSM303A_SENSITIVITY_FIFO;
    }
    else
    {
      switch(ctrl[0] & LSM303A_CTRL_REG4_FULLSCALE_MASK)
      {
        case LSM303A_CTRL_REG4_FULLSCALE_2G:
          sens = LSM303A_SENSITIVITY_2G;
          break;
        case LSM303A_CTRL_REG4_FULLSCALE_4G:
          sens = LSM303A_SENSITIVITY_4G;
          break;
        case LSM303A_CTRL_REG4_FULLSCALE_8G:
          sens = LSM303A_SENSITIVITY_8G;
          break;
        case LSM303A_CTRL_REG4_FULLSCALE_16G:
          sens = LSM303A_SENSITIVITY_16G;
          break;
      }
    }
  
    if(ctrl[0] & LSM303A_CTRL_REG4_BIG_ENDIAN)
    {
    rawX = (int16_t)(((uint16_t)buffer[0] << 8) | buffer[1]);
    rawY = (int16_t)(((uint16_t)buffer[2] << 8) | buffer[3]);
    rawZ = (int16_t)(((uint16_t)buffer[4] << 8) | buffer[5]);
    }
    else
    {
    rawX = (int16_t)(((uint16_t)buffer[1] << 8) | buffer[0]);
    rawY = (int16_t)(((uint16_t)buffer[3] << 8) | buffer[2]);
    rawZ = (int16_t)(((uint16_t)buffer[5] << 8) | buffer[4]);
    }
  
    accel->x = rawX * sens / 16;
    accel->y = rawY * sens / 16;
    accel->z = rawZ * sens / 16;
  
    return true;
  }
  return false;
}

bool lsm303m_read(LSM303_VALUES* magnet)
{
  uint8_t buffer[6], ctrl, stat;
  int16_t rawX, rawY, rawZ;
  float sensXY, sensZ;
  
  if(lsm303m_readReg(LSM303M_OUT_X_H, buffer, 6) &&
     lsm303m_readReg(LSM303M_CRB_REG, &ctrl, 1) &&
     lsm303a_readReg(LSM303M_SR_REG, &stat, 1))
  {
  
    switch(ctrl & LSM303M_CRB_REG_FULLSCALE_MASK)
    {
    case LSM303M_CRB_REG_FULLSCALE_1_3GA:
      sensXY = LSM303M_SENSITIVITY_XY_1_3GA;
      sensZ = LSM303M_SENSITIVITY_Z_1_3GA;
      break;
    case LSM303M_CRB_REG_FULLSCALE_1_9GA:
      sensXY = LSM303M_SENSITIVITY_XY_1_9GA;
      sensZ = LSM303M_SENSITIVITY_Z_1_9GA;
      break;
    case LSM303M_CRB_REG_FULLSCALE_2_5GA:
      sensXY = LSM303M_SENSITIVITY_XY_2_5GA;
      sensZ = LSM303M_SENSITIVITY_Z_2_5GA;
      break;
    case LSM303M_CRB_REG_FULLSCALE_4_GA:
      sensXY = LSM303M_SENSITIVITY_XY_4GA;
      sensZ = LSM303M_SENSITIVITY_Z_4GA;
      break;
    case LSM303M_CRB_REG_FULLSCALE_4_7_GA:
      sensXY = LSM303M_SENSITIVITY_XY_4_7GA;
      sensZ = LSM303M_SENSITIVITY_Z_4_7GA;
      break;
    case LSM303M_CRB_REG_FULLSCALE_5_6_GA:
      sensXY = LSM303M_SENSITIVITY_XY_5_6GA;
      sensZ = LSM303M_SENSITIVITY_Z_5_6GA;
      break;
    case LSM303M_CRB_REG_FULLSCALE_8_1_GA:
      sensXY = LSM303M_SENSITIVITY_XY_8_1GA;
      sensZ = LSM303M_SENSITIVITY_Z_8_1GA;
      break;
    }

    rawX = (int16_t)(((uint16_t)buffer[0] << 8) + buffer[1]);
    rawY = (int16_t)(((uint16_t)buffer[2] << 8) + buffer[3]);
    rawZ = (int16_t)(((uint16_t)buffer[4] << 8) + buffer[5]);
   
    magnet->x = rawX * 1000 * sensXY;
    magnet->y = rawY * 1000 * sensXY;
    magnet->z = rawZ * 1000 * sensZ;
  
    return true;
  }
  return false;
}

bool lsm303_init(void)
{
  uint8_t div;
  uint32_t ocr;
  
  lsm303_error = false;
  
  lsm303_anglesReal.roll = 0;
  lsm303_anglesReal.pitch = 0;
  lsm303_anglesReal.yaw = 0;
  
  i2c_setup();
  if (!lsm303a_init())
  {
    return false;
  }
  if (!lsm303m_init())
  {
    return false;    
  }
#ifdef __AVR_ATmega16__
  TCCR0 = 0x00;
#endif
#ifdef __AVR_AT90CAN128__
  TCCR0A = 0x00;
#endif
  div = 0;
  do {
    ocr = (F_CPU / (1000 * prescale0[++div])) * (LSM303M_TIMER_STEP_MS / 100) - 1;
  } while (ocr > UINT8_MAX);
#ifdef __AVR_ATmega16__
  OCR0 = (uint8_t)ocr;
#endif
#ifdef __AVR_AT90CAN128__
  OCR0A = (uint8_t)ocr;
#endif
  timerConfig = (1 << WGM01) | (((div >> 2) & 1) << CS02) | (((div >> 1) & 1) << CS01) | (((div >> 0) & 1) << CS00);
  return true;
}

void lsm303_start(void)
{
#ifdef __AVR_ATmega16__
  TCCR0 = timerConfig;
  TIMSK |= (1 << OCIE0);
#endif
#ifdef __AVR_AT90CAN128__
  TCCR0A = timerConfig;
  TIMSK0 |= (1 << OCIE0A);
#endif
  TCNT0 = 0;
}

bool lsm303_get(LSM303_ANGLES* angles)
{
  LSM303_VALUES accel, magnet;
  float norm;
  float sinRoll, cosRoll, sinPitch, cosPitch, cosYaw;
  float tiltedX, tiltedY;
  
  if (lsm303a_read(&accel) &&
      lsm303m_read(&magnet))
  {
    lsm303_error = false;
    accel.x /= 100;
    accel.y /= 100;
    accel.z /= 100;
    norm = sqrt(accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
    sinRoll = accel.y / norm;
    cosRoll = sqrt(1.0 - (sinRoll * sinRoll));
    sinPitch = accel.x / norm;
    cosPitch = sqrt(1.0 - (sinPitch * sinPitch));
    tiltedX = magnet.x * cosPitch + magnet.z * sinPitch;
    tiltedY = magnet.x * sinRoll * sinPitch + magnet.y * cosRoll - magnet.z * sinRoll * cosPitch;
    cosYaw = tiltedX / sqrt(tiltedX * tiltedX + tiltedY * tiltedY);
    angles->roll = (float)(asin(sinRoll) * 180 / M_PI);
    angles->pitch = (float)(asin(sinPitch) * 180 / M_PI);
    if(tiltedY > 0)
    {
      angles->yaw = (float)(acos(cosYaw) * 180 / M_PI);
    }  
    else
    {
      angles->yaw = 360 - (float)(acos(cosYaw) * 180 / M_PI);
    }
    return true;
  }
  lsm303_error = true;
  return false;
}
