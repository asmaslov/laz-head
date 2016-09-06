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
      comport_reply_data(motor_angleRotReal, motor_angleTiltReal, motor_inPosition);
      break;
    case HEAD_CONTROL_MOVE:
      comport_reply_ack();
      motor_inPosition = false;
      angleRotSigned = ((data->angleRotH & 0x7F) << 8) | data->angleRotL;
      angleRotSigned *= ((data->angleRotH >> 7) ? -1 : 1);
      angleTiltSigned = ((data->angleTiltH & 0x7F) << 8) | data->angleTiltL;
      angleTiltSigned *= ((data->angleTiltH >> 7) ? -1 : 1);
      motor_move(angleRotSigned, angleTiltSigned);
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
