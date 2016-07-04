/**********************************************************

Connect the red wire to +5V, 
the black wire to common ground 
and the yellow sensor wire to pin #2

**********************************************************/

#include <Ticker.h>
// which pin to use for reading the sensor? can use any pin!
#define FLOWSENSORPIN 2
Ticker ticker;

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
void ISRflowreader() {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

//void useInterrupt(boolean v) {
//  if (v) {
//    // Timer0 is already used for millis() - we'll just interrupt somewhere
//    // in the middle and call the "Compare A" function above
//    OCR0A = 0xAF;
//    TIMSK0 |= _BV(OCIE0A);
//  } else {
//    // do not call the interrupt function COMPA anymore
//    TIMSK0 &= ~_BV(OCIE0A);
//  }
//}

void FMsetup() {
   Serial.begin(115200);
   Serial.print("Flow sensor test!");
   pinMode(FLOWSENSORPIN, INPUT);
   digitalWrite(FLOWSENSORPIN, HIGH);
   lastflowpinstate = digitalRead(FLOWSENSORPIN);
   ticker.attach_ms(1,ISRflowreader);
//   useInterrupt(true);
}

void FMprint()
{ 
  Serial.print("Freq: "); Serial.println(flowrate);
  Serial.print("Pulses: "); Serial.println(pulses, DEC);
  
  float liters = pulses;
  liters /= 7.5;
  liters /= 60.0;

  Serial.print(liters); Serial.println(" Liters");

  delay(100);
}
