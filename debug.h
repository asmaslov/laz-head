#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

static inline void debug(uint8_t on)
{
  if (on == 0)
  {
  #ifdef __AVR_ATmega16__
    PORTD |= (1 << PD7);
  #endif
  #ifdef __AVR_AT90CAN128__
    PORTE |= (1 << PE4);
  #endif
  }
  else
  {
  #ifdef __AVR_ATmega16__
    PORTD &= ~(1 << PD7);
  #endif
  #ifdef __AVR_AT90CAN128__
    PORTE &= ~(1 << PE4);
  #endif
  }
}

static inline void deblink(void)
{
  debug(1);
  _delay_ms(250);
  debug(0);
  _delay_ms(250);
}

static inline void debinv(void)
{
#ifdef __AVR_ATmega16__
  if (PIND & (1 << PIND7))
#endif
#ifdef __AVR_AT90CAN128__
  if (PINE & (1 << PINE4))
#endif
  {
    debug(1);
  }
  else
  {
    debug(0);
  }
}

#endif // __DEBUG_H__
