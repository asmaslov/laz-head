#include "motor.h"
#include "head_config.h"
#include "debug.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

static const uint32_t prescale1[6] = {0, 1, 8, 64, 256, 1024};
static const uint32_t prescale2[8] = {0, 1, 8, 32, 64, 128, 256, 1024};

static bool controlRotAngle = false;
static bool controlTiltAngle = false;

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
      if (controlRotAngle && (motor_angleRotReal >= angleRotPref))
      {
        motor_stopRot();
        motor_rotInPosition = true;
      }
    #ifdef MOTOR_ROT_MAX_ANGLE  
      if (motor_angleRotReal >= MOTOR_ROT_MAX_ANGLE)
      {
        motor_stopRot();       
      }
    #endif
    }
    else
    {
      motor_angleRotReal--;
      if (controlRotAngle && (motor_angleRotReal <= angleRotPref))
      {
        motor_stopRot();
        motor_rotInPosition = true;
      }
    #ifdef MOTOR_ROT_MIN_ANGLE  
      if (motor_angleRotReal <= MOTOR_ROT_MIN_ANGLE)
      {
        motor_stopRot();
      }
    #endif    
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
      if (controlTiltAngle && (motor_angleTiltReal >= angleTiltPref))
      {
        motor_stopTilt();
        motor_tiltInPosition = true;
      }
    #ifdef MOTOR_TILT_MAX_ANGLE  
      if (motor_angleTiltReal >= MOTOR_TILT_MAX_ANGLE)
      {
        motor_stopTilt();
      }
    #endif
    }
    else
    {
      motor_angleTiltReal--;
      if (controlTiltAngle && (motor_angleTiltReal <= angleTiltPref))
      {
        motor_stopTilt();
        motor_tiltInPosition = true;
      }
    #ifdef MOTOR_TILT_MIN_ANGLE  
      if (motor_angleTiltReal <= MOTOR_TILT_MIN_ANGLE)
      {
        motor_stopTilt();
      }
    #endif
    }
  }  
}

void motor_setup(void)
{
  uint8_t div;
  uint32_t ocr;
  
  motor_rotInPosition = false;
  motor_tiltInPosition = false;
  motor_rotMoving = false;
  motor_tiltMoving = false;
  motor_rotError = false;
  motor_tiltError = false;
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

#ifdef __AVR_ATmega16__
  TCCR2 = 0x00;
#endif
#ifdef __AVR_AT90CAN128__
  TCCR2A = 0x00;
#endif
  div = 0;
  do {
    ocr = (F_CPU / (1000 * prescale2[++div])) * MOTOR_TILT_TIMER_STEP_MS - 1;
  } while (ocr > UINT8_MAX);
#ifdef __AVR_ATmega16__
  OCR2 = (uint8_t)ocr;
#endif
#ifdef __AVR_AT90CAN128__
  OCR2A = (uint8_t)ocr;
#endif
  timerTiltConfig = (1 << WGM21) | (((div >> 2) & 1) << CS22) | (((div >> 1) & 1) << CS21) | (((div >> 0) & 1) << CS20);
}

void motor_moveRotAngle(int16_t deltaAngle)
{
  motor_stopRot();
  if(deltaAngle != 0)
  {
    angleRotPref = motor_angleRotReal + deltaAngle;
    timerRotStep = 0;
  #ifdef MOTOR_ROT_MIN_ANGLE  
    if (angleRotPref < MOTOR_ROT_MIN_ANGLE)
    {
      angleRotPref = motor_angleRotReal;
      motor_rotError = true;
      return;
    }
  #endif
  #ifdef MOTOR_ROT_MAX_ANGLE  
    if (angleRotPref > MOTOR_ROT_MAX_ANGLE)
    {
      angleRotPref = motor_angleRotReal;
      motor_rotError = true;
      return;
    }
  #endif
    if((angleRotPref - motor_angleRotReal) > 0)
    {
      PORTB |= MOTOR_ROT_RIGHT;
      speedRot = 1;
      motor_rotMoving = true;
      motor_rotInPosition = false;
      controlRotAngle = true;
      TCCR1B = timerRotConfig;
    #ifdef __AVR_ATmega16__
      TIMSK |= (1 << OCIE1A);
    #endif
    #ifdef __AVR_AT90CAN128__
      TIMSK1 |= (1 << OCIE1A);
    #endif
      TCNT1 = 0;
    }
    if((angleRotPref - motor_angleRotReal) < 0)
    {
      PORTB |= MOTOR_ROT_LEFT;
      speedRot = -1;
      motor_rotMoving = true;
      motor_rotInPosition = false;
      controlRotAngle = true;
      TCCR1B = timerRotConfig;
    #ifdef __AVR_ATmega16__
      TIMSK |= (1 << OCIE1A);
    #endif
    #ifdef __AVR_AT90CAN128__
      TIMSK1 |= (1 << OCIE1A);
    #endif
      TCNT1 = 0;      
    }
  }
}

void motor_moveTiltAngle(int16_t deltaAngle)
{
  motor_stopTilt();
  if(deltaAngle != 0)
  {
    angleTiltPref = motor_angleTiltReal + deltaAngle;
    timerTiltStep = 0;
  #ifdef MOTOR_TILT_MIN_ANGLE  
    if (angleTiltPref < MOTOR_TILT_MIN_ANGLE)
    {
      angleTiltPref = motor_angleTiltReal;
      motor_tiltError = true;
      return;
    }
  #endif
  #ifdef MOTOR_TILT_MAX_ANGLE  
    if (angleTiltPref > MOTOR_TILT_MAX_ANGLE)
    {
      angleTiltPref = motor_angleTiltReal;
      motor_tiltError = true;
      return;
    }
  #endif
    if((angleTiltPref - motor_angleTiltReal) > 0)
    {
      PORTB |= MOTOR_TILT_RIGHT;
      speedTilt = 1;
      motor_tiltMoving = true;
      motor_tiltInPosition = false;
      controlTiltAngle = true;
    #ifdef __AVR_ATmega16__
      TCCR2 = timerTiltConfig;
      TIMSK |= (1 << OCIE2);
    #endif
    #ifdef __AVR_AT90CAN128__
      TCCR2A = timerTiltConfig;
      TIMSK2 |= (1 << OCIE2A);    
    #endif
      TCNT2 = 0;
    }
    if((angleTiltPref - motor_angleTiltReal) < 0)
    {
      PORTB |= MOTOR_TILT_LEFT;
      speedTilt = -1;
      motor_tiltMoving = true;
      motor_tiltInPosition = false;
      controlTiltAngle = true;
    #ifdef __AVR_ATmega16__
      TCCR2 = timerTiltConfig;
      TIMSK |= (1 << OCIE2);
    #endif
    #ifdef __AVR_AT90CAN128__
      TCCR2A = timerTiltConfig;
      TIMSK2 |= (1 << OCIE2A);
    #endif
      TCNT2 = 0;    
    }    
  }
}

void motor_stopRot(void)
{
  PORTB &= ~(MOTOR_ROT_LEFT | MOTOR_ROT_RIGHT);
#ifdef __AVR_ATmega16__
  TIMSK &= ~(1 << OCIE1A);
#endif
#ifdef __AVR_AT90CAN128__
  TIMSK1 &= ~(1 << OCIE1A);
#endif
  TCCR1A = 0x00;
  TCCR1B = 0x00;
  motor_rotMoving = false;
  speedRot = 0;
  controlRotAngle = false;
  motor_rotError = false;
}

void motor_stopTilt(void)
{
  PORTB &= ~(MOTOR_TILT_LEFT | MOTOR_TILT_RIGHT);
#ifdef __AVR_ATmega16__
  TIMSK &= ~(1 << OCIE2);
  TCCR2 = 0x00;
#endif
#ifdef __AVR_AT90CAN128__
  TIMSK2 &= ~(1 << OCIE2A);
  TCCR2A = 0x00;
#endif
  motor_tiltMoving = false;
  speedTilt = 0;
  controlTiltAngle = false;
  motor_tiltError = false;
}

void motor_moveRot(uint8_t speed)
{
  motor_stopRot();
#ifdef MOTOR_ROT_MIN_ANGLE  
  if (motor_angleRotReal <= MOTOR_ROT_MIN_ANGLE)
  {
    motor_rotError = true;
    return;
  }
#endif
#ifdef MOTOR_ROT_MAX_ANGLE  
  if (motor_angleRotReal >= MOTOR_ROT_MAX_ANGLE)
  {
    motor_rotError = true;
    return;
  }
#endif
  if ((speed & 0x80) == 0)
  {
    PORTB |= MOTOR_ROT_RIGHT;
    speedRot = 1;
    motor_rotMoving = true;
    motor_rotInPosition = false;
    TCCR1B = timerRotConfig;
  #ifdef __AVR_ATmega16__
    TIMSK |= (1 << OCIE1A);
  #endif
  #ifdef __AVR_AT90CAN128__
    TIMSK1 |= (1 << OCIE1A);
  #endif
    TCNT1 = 0;
  }
  else
  {
    PORTB |= MOTOR_ROT_LEFT;
    speedRot = -1;
    motor_rotMoving = true;
    motor_rotInPosition = false;
    TCCR1B = timerRotConfig;
  #ifdef __AVR_ATmega16__
    TIMSK |= (1 << OCIE1A);
  #endif
  #ifdef __AVR_AT90CAN128__
    TIMSK1 |= (1 << OCIE1A);
  #endif
    TCNT1 = 0;     
  }
}

void motor_moveTilt(uint8_t speed)
{
  motor_stopTilt();
#ifdef MOTOR_TILT_MIN_ANGLE  
  if (motor_angleTiltReal <= MOTOR_TILT_MIN_ANGLE)
  {
    motor_tiltError = true;
    return;
  }
#endif
#ifdef MOTOR_TILT_MAX_ANGLE  
  if (motor_angleTiltReal >= MOTOR_TILT_MAX_ANGLE)
  {
    motor_tiltError = true;
    return;
  }
#endif
  if ((speed & 0x80) == 0)
  {
    PORTB |= MOTOR_TILT_RIGHT;
    speedTilt = 1;
    motor_tiltMoving = true;
    motor_tiltInPosition = false;
  #ifdef __AVR_ATmega16__
    TCCR2 = timerTiltConfig;
    TIMSK |= (1 << OCIE2);
  #endif
  #ifdef __AVR_AT90CAN128__
    TCCR2A = timerTiltConfig;
    TIMSK2 |= (1 << OCIE2A);
  #endif
    TCNT2 = 0;
  }
  else
  {
    PORTB |= MOTOR_TILT_LEFT;
    speedTilt = -1;
    motor_tiltMoving = true;
    motor_tiltInPosition = false;
  #ifdef __AVR_ATmega16__
    TCCR2 = timerTiltConfig;
    TIMSK |= (1 << OCIE2);
  #endif
  #ifdef __AVR_AT90CAN128__
    TCCR2A = timerTiltConfig;
    TIMSK2 |= (1 << OCIE2A);
  #endif
    TCNT2 = 0; 
  }
}
