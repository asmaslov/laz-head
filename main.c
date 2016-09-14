#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <math.h>

#include "head_config.h"
#include "comport.h"
#include "motor.h"
#include "lsm303.h"
#include "debug.h"

static void init_board(void)
{
  PORTA = 0x00;
  DDRA = 0x00;
  PORTB = 0x00;
  DDRB = (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3);
  PORTC = 0x00;
  DDRC = 0x00;
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

  HeadPacket *data = (HeadPacket *)args;
  switch (data->type) {
    case HEAD_CONTROL_READ:
      lsm303_get(&lsm303_anglesReal);
      comport_reply_data(motor_angleRotReal, (int16_t)ceil(lsm303_anglesReal.pitch),
                         motor_rotInPosition, motor_tiltInPosition,
                         motor_rotMoving, motor_tiltMoving,
                         motor_rotError, motor_tiltError);
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
  }
}

int main(void)
{
  cli(); 
  init_board();
  comport_setup(command_handler);
  motor_setup();
  lsm303_init();
  debug(1);
  sei();

  while(1)
  {
  }
}
