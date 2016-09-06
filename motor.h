#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <inttypes.h>
#include <stdbool.h>

#define MOTOR_ROT_TIMER_STEP_MS 1
#define MOTOR_ROT_STEPS_IN_ANGLE 200
#define MOTOR_ROT_LEFT (1 << PB3)
#define MOTOR_ROT_RIGHT (1 << PB2)
#define MOTOR_ROT_MIN_ANGLE -360
#define MOTOR_ROT_MAX_ANGLE 360

#define MOTOR_TILT_TIMER_STEP_MS 1
#define MOTOR_TILT_STEPS_IN_ANGLE 455
#define MOTOR_TILT_LEFT (1 << PB1)
#define MOTOR_TILT_RIGHT (1 << PB0)
#define MOTOR_TILT_MIN_ANGLE -360
#define MOTOR_TILT_MAX_ANGLE 360

bool motor_rotInPosition;
bool motor_tiltInPosition;
bool motor_rotMoving;
bool motor_tiltMoving;
bool motor_rotError;
bool motor_tiltError;
int16_t motor_angleRotReal;
int16_t motor_angleTiltReal;

void motor_setup(void);
void motor_moveRotAngle(int16_t deltaAngle);
void motor_moveTiltAngle(int16_t deltaAngle);
void motor_stopRot(void);
void motor_stopTilt(void);
void motor_moveRot(bool direction);
void motor_moveTilt(bool direction);

#endif // __PWM_H__
