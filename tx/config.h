#ifndef _CONFIG_H_
#define _CONFIG_H_

// 
// Settings for configuring the blippydot.
//

// nRF24L01 settings
#define BLIP_CHANNEL  	2
#define BLIP_RADDR			(byte *)"blip2"
#define BLIP_TADDR			(byte *)"blip1"

// Blippydot Protocol
typedef struct {
  uint8_t mode   : 7;
  uint8_t invert : 1;
  uint8_t id;
  uint8_t effect;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} BLIP_MSG;

// modes
#define BLIP_BY_ID   0x13
// #define BLIP_RANDOM  0x14
#define BLIP_ROW     0x15
#define BLIP_COLUMN  0x16
#define BLIP_ALL     0x17

// effects
#define BLIP_PATTERN_SOLID  0
#define BLIP_PATTERN_RANDOM 1

#endif /* _MIRF_H_ */
