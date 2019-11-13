#include <Wire.h> // I2C library, required for MLX90614
#include <SparkFunMLX90614.h> // SparkFunMLX90614 Arduino library
#define Rx 17
#define Tx 16
#include <Servo.h>

Servo left;
Servo right;

int QTIpins[] = {47, 51, 52}; // left, center, right
int QTIdurations[3];
int QTIvalues[3];

int threshold = 100;
int encountered = 0;
int bottleLocation = -1;

IRTherm therm;
float thresholdTemperature = 66.0;

void setup() {
  Serial.begin(9600);
  configureSensing();
  configureCommunication();
  setUpQTI();
}

void loop() {
  while(encountered < 5) {
    lineFollow();
    encountered++;
    if(getTemperature() < thresholdTemperature){
      digitalWrite(2,HIGH);
      bottleLocation = encountered;
      updateLCD();
      sendCharacter((char) (48+encountered));
    }
    resetLED();
  }
}

float getTemperature() {
  therm.read();
  return therm.object();
}

void configureSensing() {
  Serial3.begin(9600);
  therm.begin();
  therm.setUnit(TEMP_F);
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);
}

void updateLCD() {
  String topLine = (bottleLocation > 0) ? "COLD @ " + String(bottleLocation) : "NOT FOUND";
  Serial3.write(12);
  Serial3.write(13);
  Serial3.println(topLine);
}

void sendCharacter(char c) {
  char outgoing = c;
    Serial2.write(outgoing);
    digitalWrite(45, LOW);
    delay(500);
}

char receiveCharacter() {
  if(Serial2.available()){
    char incoming = Serial2.read();
    if(incoming == '5'){
      digitalWrite(46, LOW);
      return incoming;
    }
  }
  return '0';
}

void resetLED(){
  digitalWrite(2, LOW); 
  digitalWrite(45, HIGH);
  digitalWrite(46, HIGH);
}

void configureCommunication(){
  pinMode(45, OUTPUT);
  pinMode(46, OUTPUT);
  pinMode(5, OUTPUT);
  Serial2.begin(9600);
  Serial.begin(9600);
}

void setUpQTI(){
  left.attach(11);
  right.attach(12);
  Serial.begin(9600);
  Serial3.begin(9600);
}
  
// Method for checking QTI sensors.
void checkSensors() {
  for(int i = 0; i < sizeof(QTIpins)/2; i++) { 
    long duration = 0;                      
    pinMode(QTIpins[i], OUTPUT);                 
    digitalWrite(QTIpins[i], HIGH);           
    delay(1);                               
    pinMode(QTIpins[i], INPUT);              
    digitalWrite(QTIpins[i], LOW);             
    while(digitalRead(QTIpins[i])){
      // Wait for RC circuit to decay.            
      duration++;                          
    }
    // Convert read value to Boolean.
    QTIdurations[i] = duration;               
    QTIvalues[i] = (duration > threshold);   
  }
}    

void lineFollow(){
  left.attach(11);
  right.attach(12);
  left.write(100);
  right.write(85);
  delay(200);
  
  while(true) {
    checkSensors();
    // Turn handling: right turn.
    if (QTIvalues[0] and not QTIvalues[2]) { // if left
      left.write(80); // left backwards
      right.write(80); // right forwards
    }
    // Turn handling: left turn.
    else if (QTIvalues[2] and not QTIvalues[0]) {
      left.write(100); // left forwards
      right.write(100); // right backwards
    }
    // Stop handling: reached a hashmark. 
    else if (QTIvalues[0] and QTIvalues[1] and QTIvalues[2]) {
      left.detach();
      right.detach();
      delay(500);
      // Break out of line following.
      return;
    }
    // Nothing to do -- keep moving forward.
    else {
      left.write(100);
      right.write(85);
    } 
  }
}


  

