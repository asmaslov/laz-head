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

bool motor_inPosition;
int16_t motor_angleRotReal;
int16_t motor_angleTiltReal;

void motor_setup(void);
void motor_move(int16_t deltaAngleRot, int16_t deltaAngleTilt);

#endif // __PWM_H__
