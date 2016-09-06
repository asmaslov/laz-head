#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <inttypes.h>
#include <stdbool.h>

#include "head_config.h"
#include "comport.h"
#include "motor.h"
#include "debug.h"

static void init_board(void)
{
  PORTA = 0x00;
  DDRA = 0x00;
  PORTB = 0x00;
  DDRB = (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3);
  PORTC = 0x00;
  DDRC = 0x00;
  PORTD = (1 << PD7);
  DDRD = (1 << DDD7);
}

static void command_handler(void *args)
{
  int16_t angleRotSigned;
  int16_t angleTiltSigned;

  HeadPacket *data = (HeadPacket *)args;
  switch (data->type) {
    case HEAD_CONTROL_READ:
      comport_reply_data(motor_angleRotReal, motor_angleTiltReal,
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
        motor_moveRot(angleRotSigned);
      }
      angleTiltSigned = ((data->angleTiltH & 0x7F) << 8) | data->angleTiltL;
      angleTiltSigned *= ((data->angleTiltH >> 7) ? -1 : 1);
      if (angleTiltSigned != 0)
      {
        motor_tiltInPosition = false;
        motor_moveTilt(angleTiltSigned);
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
      switch (data->motorIndex) {
        case HEAD_MOTOR_INDEX_ROTATE:
          switch (data->motorDirection) {
            case HEAD_MOTOR_DIRECTION_LEFT:
              motor_moveRot(false);
              break;
            case HEAD_MOTOR_DIRECTION_RIGHT:
              motor_moveRot(true);
              break;
          }
          break;
        case HEAD_MOTOR_INDEX_TILT:
          switch (data->motorDirection) {
            case HEAD_MOTOR_DIRECTION_LEFT:
              motor_moveTilt(false);
              break;
            case HEAD_MOTOR_DIRECTION_RIGHT:
              motor_moveTilt(true);
              break;
          }
          break;
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
  debug(1);
  sei();

  while(1)
  {
  }
}
