#include "comport.h"
#include "debug.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <util/setbaud.h>
#include <stdlib.h>

#define RX_BUFFER_SIZE  HEAD_PACKET_LEN
#define TX_BUFFER_SIZE  HEAD_PACKET_LEN

static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint8_t tx_wr_index, tx_rd_index, tx_counter;
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_wr_index, rx_rd_index, rx_counter;
static uint8_t rx_buffer_overflow;

static HeadPacket received_message;
static ParserHandler parser_handler;
static bool packet_received, good_packet;
static uint8_t received_part_index;

static void _putchar(uint8_t c)
{
  while (tx_counter == TX_BUFFER_SIZE);
  cli();
#ifdef __AVR_ATmega16__
  if (tx_counter || ((UCSRA & (1 << UDRE)) == 0))
#endif
#ifdef __AVR_AT90CAN128__
  if (tx_counter || ((UCSR0A & (1 << UDRE)) == 0))
#endif
  {
    tx_buffer[tx_wr_index++] = c;
    if (tx_wr_index == TX_BUFFER_SIZE)
    {
      tx_wr_index = 0;
    }
    ++tx_counter;
  }
  else
  {
#ifdef __AVR_ATmega16__
    UDR = c;
#endif
#ifdef __AVR_AT90CAN128__
    UDR0 = c;
#endif
  }
  sei();
}

static uint8_t _getchar(void)
{
  uint8_t data;
  while (rx_counter == 0);
  data = rx_buffer[rx_rd_index++];
  if (rx_rd_index == RX_BUFFER_SIZE)
  {
    rx_rd_index = 0;
  }
  cli();
  --rx_counter;
  sei();
  return data;
}

#ifdef __AVR_ATmega16__
ISR(USART_TXC_vect)
#endif
#ifdef __AVR_AT90CAN128__
ISR(USART0_TX_vect)
#endif
{
  if (tx_counter)
  {
    --tx_counter;
  #ifdef __AVR_ATmega16__
    UDR = tx_buffer[tx_rd_index++];
  #endif
  #ifdef __AVR_AT90CAN128__
    UDR0 = tx_buffer[tx_rd_index++];
  #endif
    if (tx_rd_index == TX_BUFFER_SIZE)
    {
      tx_rd_index = 0;
    }
  }
}

void comport_parse(void)
{
  uint8_t rec_byte;
  uint8_t i;
  uint16_t check_crc = 0;
  
  rec_byte = _getchar();
  if (received_part_index == 9)
  {
    received_message.crcL = rec_byte;
    received_part_index = 0;
    packet_received = true;
  }
  if (received_part_index == 8)
  {
    received_message.crcH = rec_byte;
    received_part_index = 9;
  }
  if (received_part_index == 7)
  {
    received_message.speedLimitTilt = rec_byte;
    received_part_index = 8;
  } 
  if (received_part_index == 6)
  {
    received_message.speedLimitRot = rec_byte;
    received_part_index = 7;
  } 
  if (received_part_index == 5)
  {
    received_message.angleTiltL = rec_byte;
    received_part_index = 6;
  } 
  if (received_part_index == 4)
  {
    received_message.angleTiltH = rec_byte;
    received_part_index = 5;
  }
  if (received_part_index == 3)
  {
    received_message.angleRotL = rec_byte;
    received_part_index = 4;
  }
  if (received_part_index == 2)
  {
    received_message.angleRotH = rec_byte;
    received_part_index = 3;
  }
  if (received_part_index == 1)
  {
    if ((rec_byte == HEAD_CONTROL_READ) ||
        (rec_byte == HEAD_CONTROL_MOVE_ANGLE) ||
        (rec_byte == HEAD_CONTROL_STOP) ||
        (rec_byte == HEAD_CONTROL_MOVE) ||
        (rec_byte == HEAD_CONTROL_ZERO) ||
        (rec_byte == HEAD_CONTROL_FIRE))
    {
      received_message.type = rec_byte;
      received_part_index = 2;
    }
    else
    {
      received_part_index = 0;
    }
  }
  if ((received_part_index == 0) &&
      (rec_byte == HEAD_ADDR) &&
      (!packet_received))
  {    
    received_message.unit = rec_byte;
    received_part_index = 1;
  }
  if (packet_received)
  {
    packet_received = false;
    check_crc = 0;
    for (i = 0; i < HEAD_PACKET_LEN - 2; i++)
    {
      check_crc += received_message.bytes[i];
    }
    if (check_crc == received_message.crc)
    {
      good_packet = true;
    }
  }
  if (good_packet)
  {
    good_packet = false;
    comport_need_feedback = true;
    parser_handler(&received_message);
  }
}

#ifdef __AVR_ATmega16__
ISR(USART_RXC_vect)
#endif
#ifdef __AVR_AT90CAN128__
ISR(USART0_RX_vect)
#endif
{
  uint8_t status, data;
#ifdef __AVR_ATmega16__
  status = UCSRA;
  data = UDR;
  if ((status & ((1 << FE) | (1 << PE) | (1 << DOR))) == 0)
#endif
#ifdef __AVR_AT90CAN128__
  status = UCSR0A;
  data = UDR0;
  if ((status & ((1 << FE) | (1 << UPE) | (1 << DOR))) == 0)
#endif
  {
    rx_buffer[rx_wr_index++] = data;
    if (rx_wr_index == RX_BUFFER_SIZE)
    {
      rx_wr_index = 0;
    }  
    if (++rx_counter == RX_BUFFER_SIZE)
    {
      rx_counter = 0;
      rx_buffer_overflow = 1;
    }
    if(!comport_need_feedback)
    {
      comport_parse();
    }    
  }
}

void comport_setup(ParserHandler handler)
{
#ifdef __AVR_ATmega16__
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
#endif
#ifdef __AVR_AT90CAN128__
  UBRR0H = UBRRH_VALUE;
  UBRR0L = UBRRL_VALUE;
#endif
#if USE_2X
#ifdef __AVR_ATmega16__
  UCSRA |= (1 << U2X);
#endif
#ifdef __AVR_AT90CAN128__
  UCSR0A |= (1 << U2X);
#endif
#else
#ifdef __AVR_ATmega16__
  UCSRA &= ~(1 << U2X);
#endif
#ifdef __AVR_AT90CAN128__
  UCSR0A &= ~(1 << U2X);
#endif
#endif
#ifdef __AVR_ATmega16__
  UCSRB = (1 << TXEN) | (1 << RXEN) | (1 << TXCIE) | (1 << RXCIE);
  UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
#endif
#ifdef __AVR_AT90CAN128__
  UCSR0B = (1 << TXEN) | (1 << RXEN) | (1 << TXCIE) | (1 << RXCIE);
  UCSR0C = (1 << UCSZ0) | (1 << UCSZ1);
#endif
  parser_handler = handler;
  comport_is_data_to_parse = false;
  packet_received = false;
  good_packet = false;
  comport_need_feedback = false;
  received_part_index = 0;
}

void comport_reply_ack(void)
{
  HeadPacket transmitted_message;
  uint8_t i;
  
  transmitted_message.unit = PC_ADDR;
  transmitted_message.type = HEAD_REPLY_ACK;
  transmitted_message.crc = 0;
  for (i = 0; i < HEAD_PACKET_LEN - 2; i++)
  {
    _putchar(transmitted_message.bytes[i]);
    transmitted_message.crc += transmitted_message.bytes[i];
  }
  _putchar(transmitted_message.crcH);
  _putchar(transmitted_message.crcL);
  comport_need_feedback = false;
}

void comport_reply_data(int16_t angleRot, int16_t angleTilt,
                        bool rotInPosition, bool tiltInPosition,
                        bool rotMoving, bool tiltMoving,
                        bool rotError, bool tiltError,
                        bool gyroUsed, bool gyroError,
                        bool calibrateInProgress, bool needCalibrate)
{
  HeadPacket transmitted_message;
  
  transmitted_message.unit = PC_ADDR;
  transmitted_message.type = HEAD_REPLY_DATA;
  transmitted_message.angleRot = (uint16_t)(abs(angleRot) & 0x7FFF);
  transmitted_message.angleRot |= (angleRot < 0 ? (1 << 15) : 0);
  transmitted_message.angleTilt = (uint16_t)(abs(angleTilt) & 0x7FFF);
  transmitted_message.angleTilt |= (angleTilt < 0 ? (1 << 15) : 0);
  transmitted_message.rotInPosition = rotInPosition;
  transmitted_message.tiltInPosition = tiltInPosition;
  transmitted_message.rotMoving = rotMoving;
  transmitted_message.tiltMoving = tiltMoving;
  transmitted_message.rotError = rotError;
  transmitted_message.tiltError = tiltError;
  transmitted_message.gyroUsed = gyroUsed;
  transmitted_message.gyroError = gyroError;
  transmitted_message.calibrateInProgress = calibrateInProgress;
  transmitted_message.needCalibrate = needCalibrate;
  transmitted_message.crc = 0;
  _putchar(transmitted_message.unit);
  transmitted_message.crc += transmitted_message.unit;
  _putchar(transmitted_message.type);
  transmitted_message.crc += transmitted_message.type;
  _putchar(transmitted_message.angleRotH);
  transmitted_message.crc += transmitted_message.angleRotH;
  _putchar(transmitted_message.angleRotL);
  transmitted_message.crc += transmitted_message.angleRotL;
  _putchar(transmitted_message.angleTiltH);
  transmitted_message.crc += transmitted_message.angleTiltH;
  _putchar(transmitted_message.angleTiltL);
  transmitted_message.crc += transmitted_message.angleTiltL;
  _putchar(transmitted_message.speedLimitRot);
  transmitted_message.crc += transmitted_message.speedLimitRot;
  _putchar(transmitted_message.speedLimitTilt);
  transmitted_message.crc += transmitted_message.speedLimitTilt;
  _putchar(transmitted_message.crcH);
  _putchar(transmitted_message.crcL);
  comport_need_feedback = false;
}
