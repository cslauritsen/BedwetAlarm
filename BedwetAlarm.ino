#include <avr/sleep.h>
#include "pitches.h"

//                 &     4     &   1  &   2  &   3      &     4     &  1   &  2  &  3  &     4     &
//                 8,    8,    4    , r8, r4    r8,   as8,  as8,   f4     r8, r4    r4       eb4
int melody[] = {PAS5, PAS5, PDS6    , 0,  0,     0,  PAS5, PAS5,  PF6    , 0, 0   , 0   , PDS6      ,

//  1    &     2  &     3       &  4      &  1  &  2  &  3 ...
//  c8  c8    c4       c8     eb4        f4    r8 r4    r8 ...
  PC6, PC6,  PC6   ,  PC6  , PD6    , PDS6    , 0, 0,    0
};

//             
int rhythm[] = {8, 8, 4   , 8, 4               , 8,   8,     8,     4   , 8,   4   , 4,        4,
     8,  8,    4,      8,      4,        4,     8, 4,     8 // ...
};
/* Sleep Demo Serial
 * -----------------
 * Example code to demonstrate the sleep functions in an Arduino.
 *
 * use a resistor between RX and pin2. By default RX is pulled up to 5V
 * therefore, we can use a sequence of Serial data forcing RX to 0, what
 * will make pin2 go LOW activating INT0 external interrupt, bringing
 * the MCU back to life
 *
 * there is also a time counter that will put the MCU to sleep after 10 secs
 *
 * NOTE: when coming back from POWER-DOWN mode, it takes a bit
 *       until the system is functional at 100%!! (typically <1sec)
 *
 * Copyright (C) 2006 MacSimski 2006-12-30
 * Copyright (C) 2007 D. Cuartielles 2007-07-08 - Mexico DF
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

int wakePin = 2;                 // pin used for waking up
int alarmPin = 9;                // PWM pin to drive a piezo
int sleepStatus = 0;             // variable to store a request for sleep
int count = 0;                   // counter
bool doAlarm = false;
void wakeUpNow()        // here the interrupt is handled after wakeup
{
    // execute code here after wake-up before returning to the loop() function
    // timers and code using timers (serial.print and more...) will not work here.
    doAlarm = true;
}

void playMelody() {  
       // iterate over the notes of the melody:
    for (int thisNote = 0; thisNote < sizeof(melody)/sizeof(melody[0]); thisNote++) {
  
      // to calculate the note duration, take one second
      // divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / rhythm[thisNote];
      tone(alarmPin, melody[thisNote], noteDuration);
  
      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.05;
      delay(pauseBetweenNotes);
      // stop the tone playing:
      noTone(alarmPin);
    }
}

void setup() {
    pinMode(wakePin, INPUT_PULLUP);
    
    Serial.begin(19200);
    Serial.println("begin setup...");


    /* Now it is time to enable an interrupt. In the function call
     * attachInterrupt(A, B, C)
     * A   can be either 0 or 1 for interrupts on pin 2 or 3.  
     *
     * B   Name of a function you want to execute while in interrupt A.
     *
     * C   Trigger mode of the interrupt pin. can be:
     *             LOW        a low level trigger
     *             CHANGE     a change in level trigger
     *             RISING     a rising edge of a level trigger
     *             FALLING    a falling edge of a level trigger
     *
     * In all but the IDLE sleep modes only LOW can be used.
     */

    attachInterrupt(0, wakeUpNow, LOW); // use interrupt 0 (pin 2) and run function
    // wakeUpNow when pin 2 gets LOW
    
    Serial.println("end setup...");
}

void loop() {
    // display information about the counter
    Serial.println("loop() enter");
    // compute the serial input
    if (Serial.available()) {
        int val = Serial.read();
        if (val == 'S') {
            Serial.println("Serial: Entering Sleep mode");
            delay(100);     // this delay is needed, the sleep
            //function will provoke a Serial error otherwise!!
            count = 0;
            alarmStop();
            sleepNow();     // sleep function called here
        }
        if (val == 'A') {
            Serial.println("Hola Caracola"); // classic dummy message
        }
    }
    
    if (doAlarm) {
      alarmStart();
    }
    else {
      sleepNow();
    }
}

void alarmStart() {
    playMelody();
}

void alarmStop() {
  doAlarm=false;
  analogWrite(alarmPin, 0);
}



void sleepNow()         // here we put the arduino to sleep
{
    /* Now is the time to set the sleep mode. In the Atmega8 datasheet
     * http://www.atmel.com/dyn/resources/prod_documents/doc2486.pdf on page 35
     * there is a list of sleep modes which explains which clocks and
     * wake up sources are available in which sleep mode.
     *
     * In the avr/sleep.h file, the call names of these sleep modes are to be found:
     *
     * The 5 different modes are:
     *     SLEEP_MODE_IDLE         -the least power savings
     *     SLEEP_MODE_ADC
     *     SLEEP_MODE_PWR_SAVE
     *     SLEEP_MODE_STANDBY
     *     SLEEP_MODE_PWR_DOWN     -the most power savings
     *
     * For now, we want as much power savings as possible, so we
     * choose the according
     * sleep mode: SLEEP_MODE_PWR_DOWN
     *
     */  
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here

    sleep_enable();          // enables the sleep bit in the mcucr register
    // so sleep is possible. just a safety pin

    /* Now it is time to enable an interrupt. We do it here so an
     * accidentally pushed interrupt button doesn't interrupt
     * our running program. if you want to be able to run
     * interrupt code besides the sleep function, place it in
     * setup() for example.
     *
     * In the function call attachInterrupt(A, B, C)
     * A   can be either 0 or 1 for interrupts on pin 2 or 3.  
     *
     * B   Name of a function you want to execute at interrupt for A.
     *
     * C   Trigger mode of the interrupt pin. can be:
     *             LOW        a low level triggers
     *             CHANGE     a change in level triggers
     *             RISING     a rising edge of a level triggers
     *             FALLING    a falling edge of a level triggers
     *
     * In all but the IDLE sleep modes only LOW can be used.
     */

    attachInterrupt(digitalPinToInterrupt(2),wakeUpNow, LOW); // use interrupt 0 (pin 2) and run function
    // wakeUpNow when pin 2 gets LOW

    sleep_mode();            // here the device is actually put to sleep!!
    // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP

    sleep_disable();         // first thing after waking from sleep:
    // disable sleep...
    detachInterrupt(0);      // disables interrupt 0 on pin 2 so the
    // wakeUpNow code will not be executed
    // during normal running time.

}

