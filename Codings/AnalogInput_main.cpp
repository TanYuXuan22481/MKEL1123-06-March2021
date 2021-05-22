/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "SerialStream.h"

#define BLINKING_RATE     500ms
#define MatrixRate     	  1ms
#define SLEEP_TIME                  500 // (msec)
#define PRINT_AFTER_N_LOOPS         20

// 16 point FFT
#define N		32
float   sample[2*N];
// End of FFT declaration

//Analog Input Declaration
volatile int kbhit;
AnalogIn MicIn(A0);
//End of Analog Input Declaration
 

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



// main() runs in its own thread in the OS
int main()
{

    while (true) {
        ThisThread::sleep_for(BLINKING_RATE);

		// Read ADV value
		read_adc_to_memory();
		
    }
}
