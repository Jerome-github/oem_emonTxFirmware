// **** INCLUDES *****
#include "LowPower.h"

// Use pin 3 as wake up pin
const int wakeUpPin = 3;
const int LEDpin = 9;
boolean interrupt_received = false;

void wakeUp()
{
    // Just a handler for the pin interrupt.
    interrupt_received = true;
}

void setup()
{
  pinMode(LEDpin, OUTPUT);   
  digitalWrite(LEDpin, HIGH);

    // Configure wake up pin as input.
    // This will consumes few uA of current.
    pinMode(wakeUpPin, INPUT);   
    digitalWrite(wakeUpPin, HIGH);   // and enable it's internal pull-up resistor

    digitalWrite(LEDpin, HIGH); delay(500); digitalWrite(LEDpin, LOW); delay(500);     // flash LED

  // Setup serial link
  Serial.begin(57600);
  Serial.println ("Start");
  delay(10); // without the delay, the message is lost when going to sleep
}

void loop() 
{
    static unsigned long count = 0;

    // Allow wake up pin to trigger interrupt on falling edge
    attachInterrupt(1, wakeUp, FALLING);
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(0); 
    
    // Software debounce (10 ms)
    if (interrupt_received == true){
      delay(10);
      if (digitalRead(3) == LOW){
        // Debounced interrupt action
        count++;
        Serial.println (count);
        delay(10);
      }
      interrupt_received = false;
      // Blink once per count
      for (int i=0;i<count;i++)
      {
        digitalWrite(LEDpin, HIGH); delay(100); digitalWrite(LEDpin, LOW); delay(100);     // flash LED
      }
    }
    // Blink longer.
    digitalWrite(LEDpin, HIGH); delay(500); digitalWrite(LEDpin, LOW); delay(500);     // flash LED
}

