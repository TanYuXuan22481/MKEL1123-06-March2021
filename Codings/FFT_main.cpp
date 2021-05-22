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
const static arm_cfft_instance_f32 *S;
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
    SystemReport sys_state( SLEEP_TIME * PRINT_AFTER_N_LOOPS /* Loop delay time in ms */);

    int count = 0;
	//Init arm_cfft_16
    S = & arm_cfft_sR_f32_len32;
		
    while (true) {
	
		// Read ADV value
		read_adc_to_memory();
		
		//DSP read sample
     	arm_cfft_f32(S, sample, 0, 1);
        
		// Calculate magnitude of complex numbers output by the FFT.
        arm_cmplx_mag_f32(sample, magnitudes, N);
		
		for (int i = 1; i < N; i++)
		{
			pc.printf("magnitudes[%d]*100 ~= %i \r\n", i,int(magnitudes[i]*100));
		}
		

 		//SPI_Write2(1,0x80);
		
        if ((0 == count) || (PRINT_AFTER_N_LOOPS == count)) {
            // Following the main thread wait, report on the current system status
            sys_state.report_state();
            count = 0;
        }
        ++count;
    }
}
