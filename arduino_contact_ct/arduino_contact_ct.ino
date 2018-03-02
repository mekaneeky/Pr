#include <EmonLib.h>

int sensV;
int prevset =0;
EnergyMonitor emon1;
int ESP_I;
void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  emon1.current(1, 10);
  Serial.begin(9600);
}

void loop() {
 digitalWrite(6, HIGH);
 digitalWrite(7, HIGH);
  delay(10000);

while(1){
  // put your main code here, to run repeatedly:
double Irms = emon1.calcIrms(1480);
delay(610); // to match the delays in the MCUU since we are not synced
ESP_I = (Irms*10000); //to control fidelty while still having an int
Serial.print("MCU JUST SAW: ");
Serial.print("<");
Serial.print(ESP_I);
Serial.print(">");
Serial.print("Power Estimate");
Serial.print(Irms*230.0);           // Apparent power
Serial.print("ESP_I: ");
Serial.print(ESP_I);
Serial.print("Irms ");
Serial.println(Irms);
// Irms
Serial.println(sensV);
//ON Wait OFF Pattern
delay(500);
sensV = analogRead(A0);
  if( 100>(sensV-prevset) && (sensV-prevset)>-100){
  Serial.println(sensV);
  }
  
  else if( (sensV-prevset)>100){
   prevset= turn_on();
   continue;
  }
  
  else if( -100>(sensV - prevset)){
    prevset = turn_off();
    continue;
  }
}
}
int turn_on(){
//7 is the default ON pin with relay pin IN2 (your wiring may vary)
digitalWrite(7, LOW);
delay(200);
digitalWrite(7, HIGH);
delay(3000);
Serial.println("Turning On");
return 620;
}

int turn_off() {
 //6 is the default OFF pin with relay pin IN1 (your wiring may vary)
digitalWrite(6, LOW);
delay(200);
digitalWrite(6, HIGH);
delay(1000);
Serial.println("Turning Off");
return 20; 

}
