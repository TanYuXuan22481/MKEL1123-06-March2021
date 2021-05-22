/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "stats_report.h"
#include "SerialStream.h"

#define BLINKING_RATE     500ms
#define MatrixRate     	  1ms
#define SLEEP_TIME                  500 // (msec)
#define PRINT_AFTER_N_LOOPS         20

// 16 point FFT
#define N		32
float   sample[2*N];
// End of FFT declaration
  
//8x8 LED matrix
SPI spi(D11, D12, D13);          // Arduino compatible MOSI, MISO, SCLK
//nucleo MOSI, MISO, SCLK
//      PTD2, PTD3, PTD1   
DigitalOut cs(D10);                // Chip select
//End of 8x8 LED matrix

//Serial printf
BufferedSerial serial(USBTX, USBRX, 115200);
SerialStream<BufferedSerial> pc(serial);
//End of Serial printf


/// ------------------------------------------------- MAX7219 8x8 LED Matrix Function BEGIN -----------------------------------------
/// Send two bytes to SPI bus
void SPI_Write2(unsigned char MSB, unsigned char LSB)
{
    cs = 0;                         // Set CS Low
    spi.write(MSB);                 // Send two bytes
    spi.write(LSB);
    cs = 1;                         // Set CS High
}

/// MAX7219 initialisation
void Init_MAX7219(void)
{
    SPI_Write2(0x09, 0x00);         // Decoding off
    SPI_Write2(0x0A, 0x08);         // Brightness to intermediate
    SPI_Write2(0x0B, 0x07);         // Scan limit = 7
    SPI_Write2(0x0C, 0x01);         // Normal operation mode
    SPI_Write2(0x0F, 0x0F);         // Enable display test
    ThisThread::sleep_for(BLINKING_RATE);   // 0.5 sec delay
    SPI_Write2(0x01, 0x00);         // Clear row 0.
    SPI_Write2(0x02, 0x00);         // Clear row 1.
    SPI_Write2(0x03, 0x00);         // Clear row 2.
    SPI_Write2(0x04, 0x00);         // Clear row 3.
    SPI_Write2(0x05, 0x00);         // Clear row 4.
    SPI_Write2(0x06, 0x00);         // Clear row 5.
    SPI_Write2(0x07, 0x00);         // Clear row 6.
    SPI_Write2(0x08, 0x00);         // Clear row 7.
    SPI_Write2(0x0F, 0x00);         // Disable display test
    ThisThread::sleep_for(BLINKING_RATE);   // 0.5 sec delay
}

/// 8x8 LED Matrix xlear
void Matrix_Clear(void)
{
    SPI_Write2(0x01, 0x00);         // Clear row 0.
    SPI_Write2(0x02, 0x00);         // Clear row 1.
    SPI_Write2(0x03, 0x00);         // Clear row 2.
    SPI_Write2(0x04, 0x00);         // Clear row 3.
    SPI_Write2(0x05, 0x00);         // Clear row 4.
    SPI_Write2(0x06, 0x00);         // Clear row 5.
    SPI_Write2(0x07, 0x00);         // Clear row 6.
    SPI_Write2(0x08, 0x00);         //  Clear row 7.
    SPI_Write2(0x0F, 0x00);         // Disable display test
}
/// ------------------------------------------------- MAX7219 8x8 LED Matrix Function END -----------------------------------------



// main() runs in its own thread in the OS
int main()
{
    SystemReport sys_state( SLEEP_TIME * PRINT_AFTER_N_LOOPS /* Loop delay time in ms */);

    int count = 0;
	//Init arm_cfft_16
	
	// 8x8 Matrix LED
	cs = 1;                         // CS initially High
	spi.format(8,0);                // 8-bit format, mode 0,0
	spi.frequency(1000000);         // SCLK = 1 MHz
	Init_MAX7219();                 // Initialize the LED controller
		
    while (true) {
        // Blink LED and wait 0.5 seconds
        ThisThread::sleep_for(BLINKING_RATE);
	
				
		SPI_Write2(1,0x77);
				

        if ((0 == count) || (PRINT_AFTER_N_LOOPS == count)) {
            // Following the main thread wait, report on the current system status
            sys_state.report_state();
            count = 0;
        }
        ++count;
    }
}
