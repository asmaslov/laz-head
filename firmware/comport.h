#ifndef __COMPORT_H__
#define __COMPORT_H__

#include "head_config.h"
#include "head_protocol.h"

#include <inttypes.h>
#include <stdbool.h>

#define BAUD HEAD_BAUDRATE

typedef void (*ParserHandler)(void *args);

volatile bool comport_is_data_to_parse, comport_need_feedback;

void comport_setup(ParserHandler handler);
void comport_reply_ack(void);
void comport_reply_data(int16_t angleRot, int16_t angleTilt,
                        bool rotInPosition, bool tiltInPosition,
                        bool rotMoving, bool tiltMoving,
                        bool rotError, bool tiltError,
                        bool gyroUsed, bool gyroError,
                        bool calibrateInProgress, bool needCalibrate);

#endif // __COMPORT_H__
