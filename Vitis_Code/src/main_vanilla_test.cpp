/*****************************************************************//**
 * @file main_vanilla_test.cpp
 *
 * @brief Basic test of 4 basic i/o cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

//#define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"
#include "spi_core.h"
#include "sseg_core.h"
#include "math.h"

/**
 * blink once per second for 5 times.
 * provide a sanity check for timer (based on SYS_CLK_FREQ)
 * @param led_p pointer to led instance
 */
void timer_check(GpoCore *led_p) {
   int i;

   for (i = 0; i < 5; i++) {
      led_p->write(0xffff);
      sleep_ms(500);
      led_p->write(0x0000);
      sleep_ms(500);
      debug("timer check - (loop #)/now: ", i, now_ms());
   }
}

/**
 * check individual led
 * @param led_p pointer to led instance
 * @param n number of led
 */
void led_check(GpoCore *led_p, int n) {
   int i;

   for (i = 0; i < n; i++) {
      led_p->write(1, i);
      sleep_ms(200);
      led_p->write(0, i);
      sleep_ms(200);
   }
}

/**
 * leds flash according to switch positions.
 * @param led_p pointer to led instance
 * @param sw_p pointer to switch instance
 */
void sw_check(GpoCore *led_p, GpiCore *sw_p) {
   int i, s;

   s = sw_p->read();
   for (i = 0; i < 30; i++) {
      led_p->write(s);
      sleep_ms(50);
      led_p->write(0);
      sleep_ms(50);
   }
}

/**
 * uart transmits test line.
 * @note uart instance is declared as global variable in chu_io_basic.h
 */
void uart_check() {
   static int loop = 0;

   uart.disp("uart test #");
   uart.disp(loop);
   uart.disp("\n\r");
   loop++;
}

void gsensor_check(SpiCore *spi_p, GpoCore *led_p, SsegCore *sseg_p) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t PART_ID_REG = 0x02;
   const uint8_t DATA_REG = 0x08;
   const float raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw, yraw, zraw;
   float x, y, z;
   int id;

   spi_p->set_freq(400000);
   spi_p->set_mode(0, 0);
   // check part id
   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(PART_ID_REG);  // part id address
   id = (int) spi_p->transfer(0x00);
   spi_p->deassert_ss(0);
   // read 8-bit x/y/z g values once
   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(DATA_REG);  //
   xraw = spi_p->transfer(0x00);
   yraw = spi_p->transfer(0x00);
   zraw = spi_p->transfer(0x00);
   spi_p->deassert_ss(0);
   x = (float) xraw / raw_max;
   y = (float) yraw / raw_max;
   z = (float) zraw / raw_max;
   x = round(x);
   y = round(y);
   z = round(z);

   //sseg setup
    uint8_t off = 0xff;

    for(int i = 0; i < 8; i++) {
    	sseg_p->write_1ptn(off, i);
    }

   if (x == 0 && y == -1 && z == 0) {
	   led_p->write(1, 1);
	   led_p->write(0, 0);
	   led_p->write(0, 2);
	   led_p->write(0, 3);

   } else if (x == -1 && y == 0 && z == 0) {
	   led_p->write(1, 2);
	   led_p->write(0, 0);
	   led_p->write(0, 1);
	   led_p->write(0, 3);
   } else if (x == 0 && y == 1 && z == 0) {
	   led_p->write(1, 3);
	   led_p->write(0, 0);
	   led_p->write(0, 1);
	   led_p->write(0, 2);
   } else if (x == 1 && y == 0 && z == 0){
	   led_p->write(1, 0);
	   led_p->write(0, 1);
	   led_p->write(0, 2);
	   led_p->write(0, 3);
   } else {
	   led_p->write(0, 0);
	   led_p->write(0, 1);
	   led_p->write(0, 2);
	   led_p->write(0, 3);
   }
}

// instantiate switch, led
GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
SpiCore spi(get_slot_addr(BRIDGE_BASE, S9_SPI));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));

int main() {

   while (1) {
      gsensor_check(&spi, &led, &sseg);
      debug("main - switch value / up time : ", sw.read(), now_ms());
   } //while
} //main

