#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

static inline void debug(uint8_t on)
{
  if (on == 0)
  {
    PORTD |= (1 << PORTD7);
  }
  else
  {
    PORTD &= ~(1 << PORTD7);
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
	if (PIND & (1 << PIND7))
	{
		debug(1);
	}
	else
	{
		debug(0);
	}
}

#endif // __DEBUG_H__
