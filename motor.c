#include "motor.h"
#include "head_config.h"
#include "debug.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

static const uint32_t prescale1[6] = {0, 1, 8, 64, 256, 1024};
static const uint32_t prescale2[8] = {0, 1, 8, 32, 64, 128, 256, 1024};

static volatile bool rotMoving = false;
static volatile bool tiltMoving = false;

static int16_t angleRotPref = 0;
static int16_t angleTiltPref = 0;
static int8_t speedRot = 0;
static int8_t speedTilt = 0;

static uint32_t timerRotStep = 0;
static uint8_t timerRotConfig = 0;
static uint32_t timerTiltStep = 0;
static uint8_t timerTiltConfig = 0;

ISR(TIMER1_COMPA_vect)
{
	timerRotStep++;
	if(timerRotStep > MOTOR_ROT_STEPS_IN_ANGLE)
	{
		timerRotStep = 0;
    if (speedRot >= 0)
    {
      motor_angleRotReal++;
      if (motor_angleRotReal >= angleRotPref)
      {
        PORTB &= ~(MOTOR_ROT_LEFT | MOTOR_ROT_RIGHT);
        TIMSK &= ~(1 << OCIE1A);
        TCCR1A = 0x00;
        TCCR1B = 0x00;
        rotMoving = false;
      }
    }
    else
    {
      motor_angleRotReal--;
      if (motor_angleRotReal <= angleRotPref)
      {
        PORTB &= ~(MOTOR_ROT_LEFT | MOTOR_ROT_RIGHT);
        TIMSK &= ~(1 << OCIE1A);
        TCCR1A = 0x00;
        TCCR1B = 0x00;
        rotMoving = false;
      }
    }
    if (!rotMoving && !tiltMoving)
    {
      motor_inPosition = true;
    }
	}	
}

ISR(TIMER2_COMP_vect)
{
	timerTiltStep++;
	if (timerTiltStep > MOTOR_TILT_STEPS_IN_ANGLE)
	{
		timerTiltStep = 0;
    if (speedTilt >= 0)
    {
      motor_angleTiltReal++;
      if (motor_angleTiltReal >= angleTiltPref)
      {
        PORTB &= ~(MOTOR_TILT_LEFT | MOTOR_TILT_RIGHT);
        TIMSK &= ~(1 << OCIE2);
        TCCR2 = 0x00;
        tiltMoving = false;
      }
    }
    else
    {
      motor_angleTiltReal--;
      if (motor_angleTiltReal <= angleTiltPref)
      {
        PORTB &= ~(MOTOR_TILT_LEFT | MOTOR_TILT_RIGHT);
        TIMSK &= ~(1 << OCIE2);
        TCCR2 = 0x00;
        tiltMoving = false;
      }
    }
    if (!rotMoving && !tiltMoving)
    {
      motor_inPosition = true;
    }
	}	
}

void motor_setup(void)
{
  uint8_t div;
  uint32_t ocr;
  
  motor_inPosition = false;
  motor_angleRotReal = 0;
  motor_angleTiltReal = 0;
  
  TCCR1A = 0x00;
  TCCR1B = 0x00;
  div = 0;
  do {
    ocr = (F_CPU / (1000 * prescale1[++div])) * MOTOR_ROT_TIMER_STEP_MS - 1;
  } while (ocr > UINT16_MAX);
  OCR1A = (uint16_t)ocr;
  timerRotConfig = (1 << WGM12) | (((div >> 2) & 1) << CS12) | (((div >> 1) & 1) << CS11) | (((div >> 0) & 1) << CS10);

  TCCR2 = 0x00;
  div = 0;
  do {
    ocr = (F_CPU / (1000 * prescale2[++div])) * MOTOR_TILT_TIMER_STEP_MS - 1;
  } while (ocr > UINT8_MAX);
  OCR2 = (uint8_t)ocr;
  timerTiltConfig = (1 << WGM21) | (((div >> 2) & 1) << CS22) | (((div >> 1) & 1) << CS21) | (((div >> 0) & 1) << CS20);
}

void motor_move(int16_t deltaAngleRot, int16_t deltaAngleTilt)
{
  motor_inPosition = false;
  PORTB &= ~(MOTOR_ROT_LEFT | MOTOR_ROT_RIGHT | MOTOR_TILT_LEFT | MOTOR_TILT_RIGHT);
  TIMSK &= ~((1 << OCIE1A) | (1 << OCIE2));
  TCCR1B = 0x00;
  speedRot = 0;
  rotMoving = false;
  TCCR2 = 0x00;
  speedTilt = 0;
  tiltMoving = false;
  if(deltaAngleRot != 0)
  {
    angleRotPref = motor_angleRotReal + deltaAngleRot;
    timerRotStep = 0;
  #ifdef MOTOR_ROT_MIN_ANGLE  
    if (angleRotPref < MOTOR_ROT_MIN_ANGLE)
    {
      angleRotPref = motor_angleRotReal;
    }
  #endif
  #ifdef MOTOR_ROT_MAX_ANGLE  
    if (angleRotPref > MOTOR_ROT_MAX_ANGLE)
    {
      angleRotPref = motor_angleRotReal;
    }
  #endif
    if((angleRotPref - motor_angleRotReal) > 0)
    {
      PORTB |= MOTOR_ROT_RIGHT;
      speedRot = 1;
      rotMoving = true;
      TCCR1B = timerRotConfig;
      TIMSK |= (1 << OCIE1A);
      TCNT1 = 0;
    }
    if((angleRotPref - motor_angleRotReal) < 0)
    {
      PORTB |= MOTOR_ROT_LEFT;
      speedRot = -1;
      rotMoving = true;
      TCCR1B = timerRotConfig;
      TIMSK |= (1 << OCIE1A);
      TCNT1 = 0;      
    }
  }
  if(deltaAngleTilt != 0)
  {
    angleTiltPref = motor_angleTiltReal + deltaAngleTilt;
    timerTiltStep = 0;
  #ifdef MOTOR_TILT_MIN_ANGLE  
    if (angleTiltPref < MOTOR_TILT_MIN_ANGLE)
    {
      angleTiltPref = motor_angleTiltReal;
    }
  #endif
  #ifdef MOTOR_TILT_MAX_ANGLE  
    if (angleTiltPref > MOTOR_TILT_MAX_ANGLE)
    {
      angleTiltPref = motor_angleTiltReal;
    }
  #endif
    if((angleTiltPref - motor_angleTiltReal) > 0)
    {
      PORTB |= MOTOR_TILT_RIGHT;
      speedTilt = 1;
      tiltMoving = true;
      TCCR2 = timerTiltConfig;
      TIMSK |= (1 << OCIE2);
      TCNT2 = 0;
    }
    if((angleTiltPref - motor_angleTiltReal) < 0)
    {
      PORTB |= MOTOR_TILT_LEFT;
      speedTilt = -1;
      tiltMoving = true;
      TCCR2 = timerTiltConfig;
      TIMSK |= (1 << OCIE2);
      TCNT2 = 0;    
    }    
  }
  if (!rotMoving && !tiltMoving)
  {
    motor_inPosition = true;
  }
}
