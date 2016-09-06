#ifndef __HEAD_PROTOCOL_H__
#define __HEAD_PROTOCOL_H__

#define HEAD_BAUDRATE  9600

#define HEAD_PACKET_LEN  10

#define HEAD_CONTROL_READ   0x00
#define HEAD_CONTROL_MOVE   0x01

#define HEAD_REPLY_ACK      0xFF
#define HEAD_REPLY_DATA     0xFE

#pragma anon_unions

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
      struct {
        unsigned char inPosition : 1;
        unsigned char __status_bit_1 : 1;
        unsigned char __status_bit_2 : 1;
        unsigned char __status_bit_3 : 1;
        unsigned char error : 1;
        unsigned char __status_bit_5 : 1;
        unsigned char __status_bit_6 : 1;
        unsigned char __status_bit_7 : 1;
      };
      unsigned char speedRot;
    };      
    unsigned char speedTilt;
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
