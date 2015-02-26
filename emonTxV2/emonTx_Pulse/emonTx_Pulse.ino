/*
  EmonTx Pulse example

  Many meters have pulse outputs, including electricity meters: single phase, 3-phase, 
  import, export.. Gas meters, Water flow meters, etc.

  The pulse output may be a flashing LED or a switching relay (usually solid state) or both.

  In the case of an electricity meter, a pulse output corresponds to a certain amount of
  energy passing through the meter (kWh/Wh). For single-phase domestic electricity meters
  (eg. Elster A100c), each pulse usually corresponds to 1 Wh (1000 pulses per kwh).

  The code below detects the falling edge of each pulse and increments pulseCount.
  
  Read more about pulse counting here:
  http://openenergymonitor.org/emon/buildingblocks/introduction-to-pulse-counting
 
  -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
 
  Authors: Glyn Hudson, Trystan Lea
  Builds upon JeeLabs RF12 library and Arduino

  THIS SKETCH REQUIRES:

  Libraries in the standard arduino libraries folder:
    - JeeLib    https://github.com/jcw/jeelib

  Other files in project directory (should appear in the arduino tabs above)
    - emontx_lib.ino
*/

/*Recommended node ID allocation
------------------------------------------------------------------------------------------------------------
-ID-    -Node Type- 
0       - Special allocation in JeeLib RFM12 driver - reserved for OOK use
1-4     - Control nodes 
5-10    - Energy monitoring nodes
11-14   --Un-assigned --
15-16   - Base Station & logging nodes
17-30   - Environmental sensing nodes (temperature humidity etc.)
31      - Special allocation in JeeLib RFM12 driver - Node31 can communicate with nodes on any network group
-------------------------------------------------------------------------------------------------------------
*/

#define DEBUGGING                                                       // enable this line to include debugging print statements

#define RF_freq RF12_433MHZ                                             // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                  // emonTx RFM12B node ID
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD

#define USE_UNO_BOOTLOADER                                              // Disable (comment) if you're not using the UNO bootloader (i.e using Duemilanove) - All Atmega's shipped from OpenEnergyMonitor come with Arduino Uno bootloader
#include <avr/wdt.h>                                                    // the UNO bootloader 

#define RF69_COMPAT 0                                                   // set to 1 to use RFM69CW 
#include <JeeLib.h>                                                     // make sure V12 (latest) is used if using RFM69CW
ISR(WDT_vect) { Sleepy::watchdogEvent(); }
  
typedef struct { 
  int pulse;
  } PayloadTX;

PayloadTX emontx;                                                       // neat way of packaging data for RF comms

const int pulsePin = 3;                                                 // Sensor triggers pulses on IRQ 1 = pin 3
const int LEDpin = 9;
const int debouncing_time = 10;                                         // Set interrupt debouncing time to 10 ms

// Pulse counting settings 
unsigned long pulseCount = 0;                                           // Number of pulses, used to measure energy.
unsigned long newPulse, lastPulse;                                      // Used to debounce interrupt


void setup() 
{
  Serial.begin(9600);
  Serial.println("emonTX Pulse example");
  delay(100);
             
  rf12_initialize(nodeID, RF_freq, networkGroup);                       // initialize RF
  rf12_sleep(RF12_SLEEP);

#ifdef DEBUGGING
  pinMode(LEDpin, OUTPUT);                                              // Setup indicator LED
  digitalWrite(LEDpin, LOW);
#endif
  
  // Setup pulsePin as input
  pinMode(pulsePin, INPUT);
  // Enable internal pull-up resistor if sensor between GND and output (Sleeve-Ring).
  // Comment the line if between Vcc and output (Tip-Ring).
  digitalWrite(pulsePin, HIGH);
  
  attachInterrupt(1, onPulse, FALLING);                                 // kWh interrupt attached to IRQ 1 = pin3 - hardwired to emonTx pulse jackplug. For connections see: http://openenergymonitor.org/emon/node/208
  
#ifdef USE_UNO_BOOTLOADER
  wdt_enable(WDTO_8S);
#endif
}

void loop() 
{ 
  emontx.pulse = pulseCount;
  send_rf_data();  // *SEND RF DATA* - see emontx_lib

#ifdef DEBUGGING
  Serial.println(emontx.pulse);
  digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
#endif

  emontx_sleep(10);                                                     // sleep or delay in seconds - see emontx_lib
}


// The interrupt routine - runs each time a falling edge of a pulse is detected
void onPulse()                  
{
  newPulse = micros();
  if(newPulse - lastPulse >= debouncing_time * 1000) {
    lastPulse = newPulse;                                                 
    pulseCount++;                                                         // pulseCounter               
#ifdef DEBUGGING
    digitalWrite(LEDpin, HIGH); delay(10);                               // flash LED 
    digitalWrite(LEDpin, LOW); delay(10);     
#endif
  }
}

