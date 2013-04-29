boolean interrupt_received = false;

void setup()
{

  // Setup pulse counting interrupt
  pinMode(3, INPUT);       // set interrupt0 (pin3) as input
  digitalWrite(3, HIGH);   // and enable it's internal pull-up resistor
  attachInterrupt(1, countP, FALLING);  // device signals a low when active
  delay(100);

  // Setup serial link
  Serial.begin(57600);
  Serial.println ("Start");   // signal initalization done
}

void loop()
{
  static unsigned long count = 0;
  
  // Software debounce (10 ms)
  if (interrupt_received == true){
    delay(10);
    if (digitalRead(3) == LOW){
      // Debounced interrupt action
      count++;
      Serial.println (count);
    }
    interrupt_received = false;
  }
}

void countP()
{
  interrupt_received = true;
}

