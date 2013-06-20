#include "LowPower.h"
#include <JeeLib.h>

#define freq RF12_433MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 12;                                                  // emonTx RFM12B node ID
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD needs to be same as emonBase and emonGLCD

typedef struct { int pulse;} PayloadTX;
PayloadTX emontx;                                                     // neat way of packaging data for RF comms

const int PulsePin = 3;
const int LEDpin = 9;
boolean interrupt_received = false;

void Pulse()
{
    // Just a handler for the pin interrupt.
    interrupt_received = true;
}

void setup()
{
    // Configure Pulse pin as input.
    pinMode(PulsePin, INPUT);   
    digitalWrite(PulsePin, HIGH);   // and enable it's internal pull-up resistor

    // Configure LED and flash
    pinMode(LEDpin, OUTPUT);   
    digitalWrite(LEDpin, HIGH); delay(500); digitalWrite(LEDpin, LOW); delay(500);

    // Initialize RF
    rf12_initialize(nodeID, freq, networkGroup);
    rf12_sleep(RF12_SLEEP);

    // Setup serial link
    Serial.begin(57600);
    Serial.println ("emonTX Pulse");
    delay(10); // without the delay, the message is lost when going to sleep
}

void loop() 
{
    static unsigned char count = 0;

    // Allow wake up pin to trigger interrupt on falling edge
    attachInterrupt(1, Pulse, FALLING);
    
    // Enter power down state with ADC and BOD module disabled
    // Wake up when Pulse pin is low or watchdog time elapsed
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    
    // [...] Wait for pulse interrupt or end of watchdog time
    
    // Disable external pin interrupt on Pulse pin
    detachInterrupt(0); 
    
    // If interrupt received, increment counter
    if (interrupt_received == true){
      // Software debounce (10 ms)
      delay(10);
      if (digitalRead(3) == LOW){
        count++;
        Serial.println (count);
        delay(10);
      }
      interrupt_received = false;
    }
    
    // Whether or not an interrupt was received, send count to emonBase
    emontx.pulse = count;
    send_rf_data();
    // Blink once per count
    for (int i=0;i<count;i++)
    {
      digitalWrite(LEDpin, HIGH); delay(100); digitalWrite(LEDpin, LOW); delay(100);     // flash LED
    }
    // Reset count
    count = 0;
}

void send_rf_data()
{
  rf12_sleep(RF12_WAKEUP);
  // if ready to send + exit loop if it gets stuck as it seems too
  int i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}
  rf12_sendStart(0, &emontx, sizeof emontx);
  // set the sync mode to 2 if the fuses are still the Arduino default
  // mode 3 (full powerdown) can only be used with 258 CK startup fuses
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
}

