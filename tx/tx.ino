/*
 Arduino nRF24L01 RX Test
 Version: 0.2
 Author: Alex from Inside Gadgets (http://www.insidegadgets.com)
 Created: 15/08/2012
 Last Modified: 27/03/2013
 
 Test the receiver on an nRF24L01 with the Arduino
 
 VCC - 3.3V, CE - Pin 2, CSN - Pin 10
 SCK - Pin 13, MI - Pin 12, MO - Pin 11
 
 */

#include <avr/io.h>
#include <util/delay.h>
#include <SPI.h>
#include "config.h"
#include "mirf.h"
#include "nRF24L01.h"

// Defines for setting the MiRF registers for transmitting or receiving mode
#define TX_POWERUP mirf_config_register(CONFIG, mirf_CONFIG | ( (1<<PWR_UP) | (0<<PRIM_RX) ) )
#define RX_POWERUP mirf_config_register(CONFIG, mirf_CONFIG | ( (1<<PWR_UP) | (1<<PRIM_RX) ) )

// Flag which denotes transmitting or receiving mode
volatile uint8_t PMODE;

// Write data using SPI
void spi_write_data(uint8_t * dataout, uint8_t len) {
  uint8_t i;
  for (i = 0; i < len; i++) {
    SPI.transfer(dataout[i]);
  }
}

// Read data using SPI
void spi_read_data(uint8_t * datain, uint8_t len) {
  uint8_t i;
  for (i = 0; i < len; i++) {
    datain[i] = SPI.transfer(0x00);
  }
}

// Initializes pins ans interrupt to communicate with the MiRF module
// Should be called in the early initializing phase at startup.
void mirf_init(void) {
  // Define CSN and CE as Output and set them to default
  pinMode (CSN, OUTPUT);
  pinMode (CE, OUTPUT);

  mirf_CE_lo;
  mirf_CSN_hi;
}

// Sets the important registers in the MiRF module and powers the module
// in receiving mode
void mirf_config(void) {
  // Set RF channel
  mirf_config_register(RF_CH, BLIP_CHANNEL);

  // Turn off auto-acknowledgement
  mirf_config_register(SETUP_RETR, 0);
  mirf_config_register(EN_AA, 0);
	
  // Set TX addr
  mirf_write_register(TX_ADDR, BLIP_TADDR, 5);

  // Power up in transmitter mode
  PMODE = TXMODE;
  TX_POWERUP;
}


// Flush RX and TX FIFO
void mirf_flush_rx_tx(void) {
  mirf_CSN_lo; // Pull down chip select
  SPI.transfer(FLUSH_RX); // Flush RX
  mirf_CSN_hi; // Pull up chip select

  mirf_CSN_lo; // Pull down chip select
  SPI.transfer(FLUSH_TX);  // Write cmd to flush tx fifo
  mirf_CSN_hi; // Pull up chip select
}

// Read the status register
uint8_t mirf_status(void) {
  mirf_CSN_lo; // Pull down chip select
  SPI.transfer(R_REGISTER | (REGISTER_MASK & STATUS));
  uint8_t status =  SPI.transfer(NOP); // Read status register
  mirf_CSN_hi; // Pull up chip select
  return status;
}

// Checks if data is available for reading
uint8_t mirf_data_ready(void) {
  mirf_CSN_lo;                                // Pull down chip select
  SPI.transfer(R_REGISTER | (REGISTER_MASK & STATUS));
  uint8_t status = SPI.transfer(0x00);        // Read status register
  mirf_CSN_hi;                                // Pull up chip select
  return status & (1<<RX_DR);
}

// Reads a BLIP_MSG struct.
void mirf_get_data(BLIP_MSG *msg) {
  mirf_CSN_lo;                           // Pull down chip select
  SPI.transfer(R_RX_PAYLOAD);            // Send cmd to read rx payload
  spi_read_data((uint8_t *)msg, sizeof(BLIP_MSG));  // Read payload
  mirf_CSN_hi;                               // Pull up chip select
  mirf_config_register(STATUS,(1<<RX_DR));   // Reset status register
}

// Sends a data package to the default address. Be sure to send the correct
// amount of bytes as configured as payload on the receiver.
void mirf_send(uint8_t *value, uint8_t len) {
  TX_POWERUP; // Power up

  mirf_CSN_lo; // Pull down chip select
  SPI.transfer(FLUSH_TX);  // Write cmd to flush tx fifo
  mirf_CSN_hi; // Pull up chip select

  mirf_CSN_lo;  // Pull down chip select
  SPI.transfer(W_TX_PAYLOAD); // Write cmd to write payload
  spi_write_data(value, len); // Write payload
  mirf_CSN_hi; // Pull up chip select

  mirf_CE_hi; // Start transmission
  _delay_us(15);
  mirf_CE_lo;
}

// Write one byte into the given MiRF register
void mirf_config_register(uint8_t reg, uint8_t value) {
  mirf_CSN_lo;
  SPI.transfer(W_REGISTER | (REGISTER_MASK & reg));
  SPI.transfer(value);
  mirf_CSN_hi;
}

// Reads an array of bytes from the given start position in the MiRF registers.
void mirf_read_register(uint8_t reg, uint8_t *value, uint8_t len) {
  mirf_CSN_lo;
  SPI.transfer(R_REGISTER | (REGISTER_MASK & reg));
  spi_read_data(value, len);
  mirf_CSN_hi;
}

// Writes an array of bytes into inte the MiRF registers
void mirf_write_register(uint8_t reg, uint8_t *value, uint8_t len) {
  mirf_CSN_lo;
  SPI.transfer(W_REGISTER | (REGISTER_MASK & reg));
  spi_write_data(value, len);
  mirf_CSN_hi;
}

void setup() {
  Serial.begin(9600);
  Serial.println("START");
  SPI.begin();
  mirf_init();
  _delay_ms(50);
  mirf_config(); 
}
void loop() {
  BLIP_MSG msg = {0, 0, 0, 0, 0, 0, 0};

  while(1) {
      msg.mode = BLIP_BY_ID;
      msg.invert = 0;
      msg.id = 0;
      msg.effect = BLIP_PATTERN_SOLID;
      msg.red = random(0, 0xFF);
      msg.blue = random(0, 0xFF);
      msg.green = random(0, 0xFF);

      mirf_flush_rx_tx(); // Flush TX/RX
      mirf_send((uint8_t *)&msg, sizeof(msg));
      _delay_ms(1000);
  }
}


