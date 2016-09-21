#ifndef __HEAD_PROTOCOL_H__
#define __HEAD_PROTOCOL_H__

#define HEAD_BAUDRATE  9600

#define HEAD_PACKET_LEN  10

#define HEAD_CONTROL_READ        0x00
#define HEAD_CONTROL_MOVE_ANGLE  0x01
#define HEAD_CONTROL_STOP        0x02
#define HEAD_CONTROL_MOVE        0x03
#define HEAD_CONTROL_ZERO        0x04
#define HEAD_CONTROL_FIRE        0x05

#define HEAD_MOTOR_INDEX_ROTATE  0x00
#define HEAD_MOTOR_INDEX_TILT    0x01

#define HEAD_REPLY_ACK   0xFF
#define HEAD_REPLY_DATA  0xFE

typedef union {
  unsigned char bytes[HEAD_PACKET_LEN];
  struct {
    unsigned char unit;
    unsigned char type;
    union {
      unsigned short angleRot;
      struct {
        unsigned char angleRotL;
        unsigned char angleRotH;
      };
    };
    union {
      unsigned short angleTilt;
      struct {
        unsigned char angleTiltL;
        unsigned char angleTiltH;
      };
    };
    union {
      unsigned char motorIndex;
      struct {
        unsigned char triggerActivate : 1;
        unsigned char triggerFire : 1;
        unsigned char __fire_bit_2 : 1;
        unsigned char __fire_bit_3 : 1;
        unsigned char __fire_bit_4 : 1;
        unsigned char __fire_bit_5 : 1;
        unsigned char __fire_bit_6 : 1;
        unsigned char __fire_bit_7 : 1;
      };
      struct {
        unsigned char rotInPosition : 1;
        unsigned char tiltInPosition : 1;
        unsigned char rotMoving : 1;
        unsigned char tiltMoving : 1;
        unsigned char rotError : 1;
        unsigned char tiltError : 1;
        unsigned char gyroUsed : 1;
        unsigned char __status_bit_7 : 1;
      };
      unsigned char speedLimitRot;
    };
    union {
      unsigned char motorSpeed;
      unsigned char speedLimitTilt;
    };  
    union {
      unsigned short crc;
      struct {
        unsigned char crcL;
        unsigned char crcH;
      };
    };
  };  
} HeadPacket;

#endif // __HEAD_PROTOCOL_H__
