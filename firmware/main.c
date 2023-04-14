#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <math.h>
#include <util/delay.h>

#include "head_config.h"
#include "comport.h"
#include "motor.h"
#include "lsm303.h"
#include "debug.h"

volatile bool first = true;
volatile bool calibrateInProgress = false;

static void init_board(void)
{
  PORTA = 0x00;
  DDRA = 0x00;
  PORTB = 0x00;
  DDRB = (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3);
  PORTC = 0x00;
  DDRC = (1 << DDC6) | (1 << DDC7);
#ifdef __AVR_ATmega16__
  PORTD = (1 << PD7);
  DDRD = (1 << DDD7);
#endif
#ifdef __AVR_AT90CAN128__
  PORTD = 0x00;
  DDRD = 0x00;
  PORTE = (1 << PE4);
  DDRE = (1 << DDE4);
  PORTF = 0x00;
  DDRF = 0x00;
#endif
}

static void command_handler(void *args)
{
  int16_t angleRotSigned;
  int16_t angleTiltSigned;
  uint32_t i, delay;

  HeadPacket *data = (HeadPacket *)args;
  switch (data->type) {
    case HEAD_CONTROL_READ:
    #ifdef HEAD_GYROSCOPE_REALTIME
      comport_reply_data(motor_angleRotReal, -(int16_t)floor((&lsm303_anglesReal)->pitch),
                         motor_rotInPosition, motor_tiltInPosition,
                         motor_rotMoving, motor_tiltMoving,
                         motor_rotError, motor_tiltError,
                         lsm303_used, lsm303_error,
                         calibrateInProgress, first);
    #else
      comport_reply_data(motor_angleRotReal, motor_angleTiltReal,
                         motor_rotInPosition, motor_tiltInPosition,
                         motor_rotMoving, motor_tiltMoving,
                         motor_rotError, motor_tiltError,
                         lsm303_used, lsm303_error,
                         calibrateInProgress, first);
    #endif
      break;
    case HEAD_CONTROL_MOVE_ANGLE:
      comport_reply_ack();
      angleRotSigned = ((data->angleRotH & 0x7F) << 8) | data->angleRotL;
      angleRotSigned *= ((data->angleRotH >> 7) ? -1 : 1);
      if (angleRotSigned != 0)
      {
        motor_rotInPosition = false;
        motor_moveRotAngle(angleRotSigned);
      }
      angleTiltSigned = ((data->angleTiltH & 0x7F) << 8) | data->angleTiltL;
      angleTiltSigned *= ((data->angleTiltH >> 7) ? -1 : 1);
      if (angleTiltSigned != 0)
      {
        motor_tiltInPosition = false;
        motor_moveTiltAngle(angleTiltSigned);
      }
      break;
    case HEAD_CONTROL_STOP:
      comport_reply_ack();
      switch (data->motorIndex) {
        case HEAD_MOTOR_INDEX_ROTATE:
          motor_stopRot();
          break;
        case HEAD_MOTOR_INDEX_TILT:
          motor_stopTilt();
          break;
      }        
      break;
    case HEAD_CONTROL_MOVE:
      comport_reply_ack();
      if((data->motorSpeed & 0x7F) != 0)
      {
        switch (data->motorIndex) {
          case HEAD_MOTOR_INDEX_ROTATE:
            motor_moveRot(data->motorSpeed);
            break;
          case HEAD_MOTOR_INDEX_TILT:
            motor_moveTilt(data->motorSpeed);
            break;
        }
      }                  
      break;
    case HEAD_CONTROL_ZERO:
      comport_reply_ack();
      calibrateInProgress = true;
      if (lsm303_used && lsm303_get(&lsm303_anglesReal))
      {
        motor_angleTiltReal = -(int16_t)floor(lsm303_anglesReal.pitch);
        motor_moveTiltAngle(-motor_angleTiltReal);
        if (first)
        {
          debug(1);
          motor_angleRotReal = MOTOR_ROT_MAX_ANGLE + 3 * MOTOR_ROT_GAP;
          motor_moveRotAngle(-(MOTOR_ROT_MAX_ANGLE - MOTOR_ROT_MIN_ANGLE) - 2 * MOTOR_ROT_GAP);
          while(!motor_rotInPosition || !motor_tiltInPosition);
          motor_angleRotReal = MOTOR_ROT_MIN_ANGLE - MOTOR_ROT_GAP;
          motor_moveRotAngle(-motor_angleRotReal);
          while(!motor_rotInPosition);
          debug(0);
        }
        else
        {
          motor_moveRotAngle(-motor_angleRotReal);
          while(!motor_rotInPosition || !motor_tiltInPosition);    
        }
      }
      else
      {
        if (first)
        {
          debug(1);
          motor_angleTiltReal = MOTOR_TILT_MAX_ANGLE + 3 * MOTOR_TILT_GAP;    
          motor_moveTiltAngle(-(MOTOR_TILT_MAX_ANGLE - MOTOR_TILT_MIN_ANGLE) - 2 * MOTOR_TILT_GAP);
          motor_angleRotReal = MOTOR_ROT_MAX_ANGLE + 3 * MOTOR_ROT_GAP;
          motor_moveRotAngle(-(MOTOR_ROT_MAX_ANGLE - MOTOR_ROT_MIN_ANGLE) - 2 * MOTOR_ROT_GAP);
          while(!motor_rotInPosition || !motor_tiltInPosition);
          motor_angleTiltReal = MOTOR_TILT_MIN_ANGLE - MOTOR_TILT_GAP;    
          motor_moveTiltAngle(-motor_angleTiltReal);
          motor_angleRotReal = MOTOR_ROT_MIN_ANGLE - MOTOR_ROT_GAP;
          motor_moveRotAngle(-motor_angleRotReal);
          while(!motor_rotInPosition || !motor_tiltInPosition)
          debug(0);
        }
        else
        {
          motor_moveTiltAngle(-motor_angleTiltReal);
          motor_moveRotAngle(-motor_angleRotReal);
          while(!motor_rotInPosition || !motor_tiltInPosition);
        }                
      }
      first = false;
      motor_angleRotReal = 0;
      motor_angleTiltReal = 0;
      calibrateInProgress = false;
      break;
    case HEAD_CONTROL_FIRE:
      comport_reply_ack();
      if (data->triggerActivate)
      {
        PORTC |= HEAD_FIRE_TRIGGER_ACTIVATE;
      }
      else
      {
        PORTC &= ~HEAD_FIRE_TRIGGER_ACTIVATE;
      }
      if (data->triggerFire)
      {
        PORTC |= HEAD_FIRE_TRIGGER_FIRE;
      }
      else
      {
        PORTC &= ~HEAD_FIRE_TRIGGER_FIRE;
      }
      if (data->triggerSingle)
      {
        PORTC |= HEAD_FIRE_TRIGGER_ACTIVATE;
        delay = HEAD_FIRE_SINGLE_ACTIVATE_DELAY_MS;
        for (i = 0; i < delay; i++)
        {
          _delay_ms(10);
          wdt_reset();
        }
        PORTC |= HEAD_FIRE_TRIGGER_FIRE;
        delay = HEAD_FIRE_SINGLE_FIRE_TIME_MS;
        for (i = 0; i < delay; i++)
        {
          _delay_ms(10);
          wdt_reset();
        }
        PORTC &= ~(HEAD_FIRE_TRIGGER_ACTIVATE | HEAD_FIRE_TRIGGER_FIRE);
      }
      break;
  }
}

int main(void)
{
  cli();
  wdt_disable();
  init_board();
  comport_setup(command_handler);
  motor_setup();
  lsm303_used = lsm303_init();
  if (lsm303_used && lsm303_get(&lsm303_anglesReal))
  {
    motor_angleTiltReal = (int16_t)floor(lsm303_anglesReal.pitch);
  #ifdef HEAD_BOOT_BLINK
    deblink(3);
  }
  else
  {
    deblink(2);
  #endif
  }
  sei();
  wdt_enable(WDTO_500MS);
#ifdef HEAD_GYROSCOPE_REALTIME
  if (lsm303_used)
  {
    lsm303_start();
  }
#endif
  while(1)
  {
    wdt_reset();
  }
}
