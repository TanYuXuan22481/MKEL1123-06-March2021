/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"
#include "SerialStream.h"

#define BLINKING_RATE     500ms
#define SLEEP_TIME                  500 // (msec)
#define PRINT_AFTER_N_LOOPS         20

//Serial printf
BufferedSerial serial(USBTX, USBRX, 115200);
SerialStream<BufferedSerial> pc(serial);
//End of Serial printf


// main() runs in its own thread in the OS
int main()
{
    while (true) {
    ThisThread::sleep_for(BLINKING_RATE);

	pc.printf("Hello World! \r\n");
	pc.printf("This is one = %d! \r\n",1);
    }
}
