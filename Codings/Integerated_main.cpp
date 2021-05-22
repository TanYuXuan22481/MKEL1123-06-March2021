/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include <arm_math.h>
#include "mbed.h"
#include "stats_report.h"
#include "SerialStream.h"
#include <arm_const_structs.h>

#define BLINKING_RATE     500ms
#define MatrixRate     	  1ms
#define SLEEP_TIME                  500 // (msec)
#define PRINT_AFTER_N_LOOPS         20

// 16 point FFT
#define N		32
float   sample[2*N];
float   magnitudes[N];
uint8_t led_buf[8];
const static arm_cfft_instance_f32 *S;
// End of FFT declaration

//Analog Input Declaration
volatile int kbhit;
AnalogIn MicIn(A0);
//End of Analog Input Declaration

//Blinking LED
DigitalOut led1(LED1);
//End of Blinking LED
  
//8x8 LED matrix
SPI spi(D11, D12, D13);          // Arduino compatible MOSI, MISO, SCLK
//nucleo MOSI, MISO, SCLK
//      PTD2, PTD3, PTD1   
DigitalOut cs(D10);                // Chip select

const unsigned char led2[]= {
    0xFF,0x18,0x18,0x18,0x18,0x18,0x18,0xFF
};  //H
const unsigned char led3[]= {
    0x1F,0x60,0x80,0x40,0x40,0x80,0x60,0x1F
};  //W
//End of 8x8 LED matrix

//Serial printf
BufferedSerial serial(USBTX, USBRX, 115200);
SerialStream<BufferedSerial> pc(serial);
//End of Serial printf

void vUpdate(void){
    kbhit = 1;
}

/// ------------------------------------------------- ADC VALUE BEGIN -----------------------------------------
void read_adc_to_memory()
{
    Ticker Period;
    kbhit = 0;   
    Period.attach_us(&vUpdate,25);
    //int harvest=0;
    for (int i = 0; i < N*2 ; i+=2)
    //for (int i=0; i < N*2 ; i++)
    {  
        while(kbhit==0);
        kbhit=0;
	//harvest = MicIn.read_u16();
        sample[i] = MicIn.read();//buffer[i];
        sample[i+1] = 0;
		
		pc.printf("sample[%d]*100 ~= %i \r\n", i,int(sample[i]*100));

    }
    
    Period.detach();
}
/// ------------------------------------------------- ADC VALUE END -----------------------------------------


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
    SPI_Write2(0x08, 0x00);         // Clear row 7.
    SPI_Write2(0x0F, 0x00);         // Disable display test
}

void update_8x8_led(){

int SCAN_FREQUENCY = 25;

    for (int scanning_freq = 0; scanning_freq < SCAN_FREQUENCY; scanning_freq++){
        for (int j = 1; j < 9; j++) {
            //ground latchPin and hold low for as long as you are transmitting
			//shiftOut(latchPinRow, dataPin, clockPin, 1  << j);
    	    //shiftOut(latchPinCol, dataPin, clockPin, 0xff-led_buf[j]);
			//SPI_Write2(row,col);
            SPI_Write2(j,led_buf[j-1]);
		ThisThread::sleep_for(MatrixRate);   // 0.1 sec delay 
		}  
    }
}
/// ------------------------------------------------- MAX7219 8x8 LED Matrix Function END -----------------------------------------

/// ------------------------------------------------- MAX7219 8x8 LED Matrix Data Preparation BEGIN -----------------------------------------
void mag_to_buf()
{
    
    float xmax = 0;
    int   temp = 0;
    float temp_float = 0;
    float logged[N];
    int led = 0;
 //  for (int i = 0; i < N; i++)
 //  {		
       //logged[i] = (float)log( 30.0 * (double) magnitudes[i] + 1 );
 //  }
   

   pc.printf("magnitudes[%d]*100 ~= %i \r\n", 1,int(magnitudes[1]*100));
   
   for (int i = 1; i < N; i++)
   {
	      pc.printf("magnitudes[%d]*100 ~= %i \r\n", i,int(magnitudes[i]*100));

       if (magnitudes[i] > xmax)
       {
           xmax = magnitudes[i];
       }
   }
   
   if (magnitudes[0] - xmax > 1.0) 
   {
   
      //scaled data
       for (int i = 1 ; i < 9  ; i++)
       {   
            temp_float = magnitudes[i];
	    //if (temp_float > xmax)
	    //{
	    //    temp_float = xmax;
	    //}
            //temp = (int)pow(2,((int)magnitudes[i+2]*(N/2)/(int)xmax))-1;
	    //printf("%f\n",temp_float);
	    led = (int)(temp_float * 8.0 / (magnitudes[0]*70/100));
	    //printf("%d",led);
            if (led > 8){led = 8;}

	    temp = (int)pow(2,(double)led)-1;
	    led_buf[i-1] = temp; 
        }

    } else {
       for (int i = 0; i < 8; i++)
       {
	   led_buf[i] = 0;
       }
    }
}
/// ------------------------------------------------- MAX7219 8x8 LED Matrix Function END -----------------------------------------



// main() runs in its own thread in the OS
int main()
{
    SystemReport sys_state( SLEEP_TIME * PRINT_AFTER_N_LOOPS /* Loop delay time in ms */);

    int count = 0;
	//Init arm_cfft_16
    S = & arm_cfft_sR_f32_len32;
    //printf("Debug1\r\n");
	
	// 8x8 Matrix LED
	cs = 1;                         // CS initially High
	spi.format(8,0);                // 8-bit format, mode 0,0
	spi.frequency(1000000);         // SCLK = 1 MHz
	Init_MAX7219();                 // Initialize the LED controller
		
    while (true) {
        // Blink LED and wait 0.5 seconds
        led1 = !led1;
        ThisThread::sleep_for(BLINKING_RATE);

		//Printf trying
		pc.printf("Hello World! \r\n");
	
		// Read ADV value
		read_adc_to_memory();

		pc.printf("\n");
		
		
		//DSP read sample
     	arm_cfft_f32(S, sample, 0, 1);
        
		// Calculate magnitude of complex numbers output by the FFT.
        arm_cmplx_mag_f32(sample, magnitudes, N);
		
		// Calculate magnitude of complex numbers output by the FFT.
		mag_to_buf();
		
		SPI_Write2(1,0x70);
		
		//update_8x8_led();
		
		
		/* 		for(int i=1; i<9; i++)      // Write first character (8 rows)
			SPI_Write2(i,led2[i-1]);
		ThisThread::sleep_for(BLINKING_RATE);   // 0.5 sec delay
		for(int i=1; i<9; i++)      // Write second character
			SPI_Write2(i,led3[i-1]);
		ThisThread::sleep_for(BLINKING_RATE);   // 0.5 sec delay */
		
		//Matrix_Clear();
 		//SPI_Write2(1,0x80);
		
        if ((0 == count) || (PRINT_AFTER_N_LOOPS == count)) {
            // Following the main thread wait, report on the current system status
            sys_state.report_state();
            count = 0;
        }
        ++count;
    }
}
