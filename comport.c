#include "comport.h"
#include "debug.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <util/setbaud.h>

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
  if (tx_counter || ((UCSRA & (1 << UDRE)) == 0))
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
    UDR = c;
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

ISR(USART_TXC_vect)
{
  if (tx_counter)
  {
    --tx_counter;
    UDR = tx_buffer[tx_rd_index++];
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
    received_message.speedTilt = rec_byte;
    received_part_index = 8;
  } 
  if (received_part_index == 6)
  {
    received_message.speedRot = rec_byte;
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
        (rec_byte == HEAD_CONTROL_MOVE))
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

ISR(USART_RXC_vect)
{
  uint8_t status, data;
  status = UCSRA;
  data = UDR;
  if ((status & ((1 << FE) | (1 << PE) | (1 << DOR))) == 0)
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
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
#if USE_2X
  UCSRA |= (1 << U2X);
#else
  UCSRA &= ~(1 << U2X);
#endif
  UCSRB = (1 << TXEN) | (1 << RXEN) | (1 << TXCIE) | (1 << RXCIE);
  UCSRC = (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);
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
  
  transmitted_message.unit = HEAD_ADDR;
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

void comport_reply_data(uint16_t angleRot, uint16_t angleTilt, bool inPosition)
{
  HeadPacket transmitted_message;
  uint8_t i;
  
  transmitted_message.unit = HEAD_ADDR;
  transmitted_message.type = HEAD_REPLY_DATA;
  transmitted_message.angleRot = angleRot;
  transmitted_message.angleTilt = angleTilt;
  transmitted_message.inPosition = inPosition;
  transmitted_message.crc = 0;
  for (i = 0; i < HEAD_PACKET_LEN - 2; ++i)
  {
    _putchar(transmitted_message.bytes[i]);
    transmitted_message.crc += transmitted_message.bytes[i];
  }
  _putchar(transmitted_message.crcH);
  _putchar(transmitted_message.crcL);
  comport_need_feedback = false;
}
