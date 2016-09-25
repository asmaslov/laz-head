#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <inttypes.h>
#include <stdbool.h>

#define MOTOR_ROT_TIMER_STEP_MS 1
#define MOTOR_ROT_STEPS_IN_ANGLE 200
#define MOTOR_ROT_LEFT (1 << PB2)
#define MOTOR_ROT_RIGHT (1 << PB3)
#define MOTOR_ROT_MIN_ANGLE -45
#define MOTOR_ROT_MAX_ANGLE 45
#define MOTOR_ROT_GAP 5

#define MOTOR_TILT_TIMER_STEP_MS 1
#define MOTOR_TILT_STEPS_IN_ANGLE 455
#define MOTOR_TILT_LEFT (1 << PB1)
#define MOTOR_TILT_RIGHT (1 << PB0)
#define MOTOR_TILT_MIN_ANGLE -38
#define MOTOR_TILT_MAX_ANGLE 38
#define MOTOR_TILT_GAP 5

volatile bool motor_rotInPosition;
volatile bool motor_tiltInPosition;
volatile bool motor_rotMoving;
volatile bool motor_tiltMoving;
bool motor_rotError;
bool motor_tiltError;
int16_t motor_angleRotReal;
int16_t motor_angleTiltReal;

void motor_setup(void);
void motor_moveRotAngle(int16_t deltaAngle);
void motor_moveTiltAngle(int16_t deltaAngle);
void motor_stopRot(void);
void motor_stopTilt(void);
void motor_moveRot(uint8_t speed);
void motor_moveTilt(uint8_t speed);

#endif // __PWM_H__
