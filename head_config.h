#ifndef __HEAD_CONFIG_H__
#define __HEAD_CONFIG_H__

#ifndef F_CPU
  #define F_CPU  16000000
#endif

#define HEAD_ADDR  0x21
#define PC_ADDR    0xA1

#define HEAD_ROTATE_RANGE_ANGLE  60
#define HEAD_TILT_RANGE_ANGLE    84

//#define HEAD_GYROSCOPE_REALTIME

#define HEAD_FIRE_TRIGGER_ACTIVATE  (1 << PC6)
#define HEAD_FIRE_TRIGGER_FIRE      (1 << PC7)

#endif // __HEAD_CONFIG_H__
