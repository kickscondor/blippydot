/*
 * Blippydot RX (Receiver - attached to the RGB LED)
 *
 * Based on code by Alex from insideGadgets (http://www.insidegadgets.com)
 *
 */
 
// ATtiny24/44/84 Pin map
//
//                     +-\/-+
//       3.3v -> VCC  1|o   |14  GND -> GND
//         CE -> PB0  2|    |13  PA0 -> LED RED
//        CSN -> PB1  3|    |12  PA1
//      RESET -> PB3  4|    |11  PA2
//   LED BLUE -> PB2  5|    |10  PA3
//  LED GREEN -> PA7  6|    |9   PA4 -> SCK
//  DI (MISO) -> PA6  7|    |8   PA5 -> DO (MOSI)
//                     +----+

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <string.h>
#include "../tx/config.h"
#include "setup.c"
#include "mirf.c"

#define redGlow   OCR1A
#define greenGlow OCR0A
#define blueGlow  OCR0B

void color(uint8_t red, uint8_t green, uint8_t blue) {
  redGlow = red;
  greenGlow = green;
  blueGlow = blue;
}

int main(void) {
	setup();
  color(0xFF, 0xFF, 0xFF);

	mirf_init();
	_delay_ms(50);
	mirf_config();
  BLIP_MSG msg = {0, 0, 0, 0, 0, 0};
	
  color(0xFF, 0, 0);
  _delay_ms(100);
  color(0, 0xFF, 0);
  _delay_ms(100);
  color(0, 0, 0xFF);
  _delay_ms(100);

  mirf_CE_hi;
	while(1) {
    bool q = false;

    mirf_flush_rx_tx(); // Flush TX/RX

    while (!mirf_data_ready()) {
      _delay_ms(50);
    }

    mirf_get_data(&msg);
    switch (msg.mode) {
      case BLIP_BY_ID:
        q = (msg.id == BLIP_ID);
      break;

      case BLIP_ROW:
        q = ((BLIP_ID / (msg.id & 0xF)) == (msg.id & 0xF0)>>4);
      break;

      case BLIP_COLUMN:
        q = ((BLIP_ID % (msg.id & 0xF)) == (msg.id & 0xF0)>>4);
      break;

      case BLIP_ALL:
        q = true;
      break;

      default:
        q = false;
    }

    if (msg.invert) q = !q;
    if (!q) continue;

    switch (msg.effect) {
      case BLIP_PATTERN_SOLID:
        color(msg.red, msg.green, msg.blue);
      break;
    }
	}
}

ISR(TIM1_COMPA_vect) {
  sbi(PORTA, RED_PIN);
}

ISR(TIM1_OVF_vect) {
  cbi(PORTA, RED_PIN);
}

ISR(WDT_vect) { }

ISR(PCINT0_vect) { }
